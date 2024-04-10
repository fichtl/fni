#include "dni/framework/graph.h"

#include <atomic>
#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "dni/framework/datum.h"
#include "dni/framework/dni.pb.h"
#include "dni/framework/graph_config.h"
#include "dni/framework/graph_output_stream.h"
#include "dni/framework/input_stream_manager.h"
#include "dni/framework/node.h"
#include "dni/framework/output_side_data.h"
#include "dni/framework/output_stream_manager.h"
#include "dni/framework/utils/tags.h"
#include "fmt/ranges.h"
#include "google/protobuf/text_format.h"
#include "spdlog/spdlog.h"
#include "taskflow/algorithm/data_pipeline.hpp"
#include "taskflow/taskflow.hpp"

namespace dni {

        Graph::Graph(GraphConfig config)
        {
                if (Initialize(std::move(config)))
                {
                        spdlog::critical("failed to initialize Graph");
                        spdlog::shutdown();
                }
        }

        int Graph::InitializeStreams()
        {
                // Initialize all input streams.
                int nstreams = cfg_->InputStreams().size();
                SPDLOG_DEBUG(
                    "initializing InputStreamManagers: [{:}]", cfg_->InputStreams());
                input_stream_managers_ = std::make_unique<InputStreamManager[]>(nstreams);
                for (int i = 0; i < nstreams; ++i)
                {
                        // SPDLOG_DEBUG(
                        //     "initializing InputStreamManager: {}",
                        //     input_stream_managers_[i].Name());
                        const EdgeInfo& edge_info = cfg_->InputStreams()[i];
                        input_stream_managers_[i].Initialize(
                            edge_info.name, edge_info.datum_type);
                        input_stream_manager_lookup_[&input_stream_managers_[i]] = i;
                }

                // Initialize all output streams (GraphInputStreams included).
                nstreams = cfg_->OutputStreams().size();
                SPDLOG_DEBUG(
                    "initializing OutputStreamManagers: [{:}]", cfg_->OutputStreams());
                output_stream_managers_ =
                    std::make_unique<OutputStreamManager[]>(nstreams);
                for (int i = 0; i < nstreams; ++i)
                {
                        // SPDLOG_DEBUG(
                        //     "initializing OutputStreamManager: {}",
                        //     output_stream_managers_[i].Name());
                        const EdgeInfo& edge_info = cfg_->OutputStreams()[i];
                        output_stream_managers_[i].Initialize(
                            edge_info.name, edge_info.datum_type);
                        output_stream_manager_lookup_[&output_stream_managers_[i]] = i;
                }

                // Initialize `GraphInputStream`s.
                int graph_input_stream_count = 0;
                std::shared_ptr<utils::TagMap> input_tag_map =
                    utils::NewTagMap(cfg_->Proto().input_stream());
                SPDLOG_DEBUG(
                    "initializing GraphInputStreams: [{:}]", input_tag_map->Names());
                for (auto stream_name : input_tag_map->Names())
                {
                        int osi = cfg_->OutputStreamIndex(stream_name);
                        if (osi < 0)
                        {
                                SPDLOG_ERROR("failed to locate graph input stream");
                                return -1;
                        }

                        const EdgeInfo& edge = cfg_->OutputStreams()[osi];
                        if (cfg_->OutputStreams()[osi].parent.type !=
                            NodeInfo::NodeType::GRAPH_INPUT_STREAM)
                        {
                                SPDLOG_ERROR("invalid graph input stream info");
                                return -1;
                        }

                        int node_idx = cfg_->Nodes().size() + graph_input_stream_count;
                        if (edge.parent.index != node_idx)
                        {
                                SPDLOG_ERROR(
                                    "ParsedGraphConfig idx({}) != calculated idx({})",
                                    edge.parent.index,
                                    node_idx);

                                return -1;   // need return -1 here ??
                        }

                        // Only initialize `GraphInputStream`s here, since its manager is
                        // already initialized (in output_stream_managers_) before.
                        graph_input_streams_[stream_name] =
                            std::make_unique<GraphInputStream>(
                                &output_stream_managers_[osi]);

                        graph_input_stream_to_node_[stream_name] = node_idx;

                        ++graph_input_stream_count;
                }

                return 0;
        }

        int Graph::InitializeNodes()
        {
                auto nstreams = cfg_->OutputSideData().size();
                SPDLOG_DEBUG(
                    "initializing OutputSideData: [{:}]", cfg_->OutputSideData());
                output_side_data_ = std::make_unique<OutputSideDatumImpl[]>(nstreams);
                for (int i = 0; i < nstreams; ++i)
                {
                        const EdgeInfo& edge_info = cfg_->OutputSideData()[i];
                        output_side_data_[i].Initialize(
                            edge_info.name, edge_info.datum_type);

                        SPDLOG_DEBUG(
                            "initializing OutputSideDatumImpl: {}",
                            output_side_data_[i].Name());
                }

                int nnodes = cfg_->Nodes().size();
                nodes_.reserve(nnodes);

                for (int i = 0; i < nnodes; ++i)
                {
                        NodeInfo::NodeRef ref(NodeInfo::NodeType::TASK, i);
                        nodes_.emplace_back(std::make_unique<Node>());
                        nodes_.back()->Initialize(
                            cfg_.get(), ref, input_stream_managers_.get(),
                            output_stream_managers_.get(), output_side_data_.get());
                }

                SPDLOG_DEBUG("nodes after initialization:\n{:}", fmt::join(nodes_, "\n"));

                return 0;
        }

        int Graph::Initialize(GraphConfig config)
        {
                return Initialize(std::move(config), {});
        }

        int Graph::Initialize(
            GraphConfig config, const std::map<std::string, Datum>& side_data)
        {
                if (initialized_)
                {
                        return 0;
                }

                SPDLOG_DEBUG("Initializing graph config");
                auto validated_config = std::make_unique<ParsedGraphConfig>();
                if (validated_config->Initialize(config))
                {
                        SPDLOG_ERROR("failed to initialize graph config from protobuf");
                        return -1;
                }
                cfg_ = std::move(validated_config);

                SPDLOG_DEBUG("Initializing streams");
                if (InitializeStreams())
                        return -1;

                SPDLOG_DEBUG("Initializing nodes");
                if (InitializeNodes())
                        return -1;

                initialized_ = true;
                return 0;
        }

        int Graph::PrepareForRun()
        {
                std::unordered_map<std::string, tf::Task> task_map;

                // init current_input_side_data_
                SPDLOG_DEBUG("Preparing current_input_side_data_");
                for (const auto& edge : cfg_->OutputSideData())
                {
                        SPDLOG_DEBUG("current side data name {}", edge.name);
                        current_input_side_data_[edge.name] = Datum();
                }

                SPDLOG_DEBUG("Preparing nodes");
                for (auto& node : nodes_)
                {
                        node->PrepareForRun(current_input_side_data_);
                }

                for (auto& graph_output : graph_output_streams_)
                {
                        graph_output->PrepareForRun();
                }

                // Open
                SPDLOG_DEBUG("Opening nodes");
                for (auto& node : nodes_)
                {
                        tf::Task tf_node =
                            taskflow_.emplace([&]() { node->Open(); })
                                .name(fmt::format(
                                    "open node {}", node->Config().NodeName()));

                        task_map.emplace(node->Config().NodeName(), tf_node);
                }
                for (auto& node : nodes_)
                {
                        auto tf_node = task_map.at(node->Config().NodeName());

                        for (auto pred : node->Predecessors())
                        {
                                auto tf_task_pred = task_map.at(pred);
                                tf_node.succeed(tf_task_pred);
                        }
                }
                SPDLOG_DEBUG("\n{}", taskflow_.dump());
                executor_.run(taskflow_).wait();
                taskflow_.clear();
                task_map.clear();

                // Process, step1
                // TODO: consider changing control flow based on return value.
                for (auto& node : nodes_)
                {
                        tf::Task tf_node = taskflow_.emplace([&]() { node->Process(); })
                                               .name(fmt::format(
                                                   "{}({})", node->Config().NodeName(),
                                                   node->State().TaskType()));

                        task_map.emplace(node->Config().NodeName(), tf_node);
                }
                for (auto& node : nodes_)
                {
                        auto tf_node = task_map.at(node->Config().NodeName());

                        for (auto pred : node->Predecessors())
                        {
                                auto tf_task_pred = task_map.at(pred);
                                tf_node.succeed(tf_task_pred);
                        }
                }
                SPDLOG_DEBUG("\n{}", taskflow_.dump());
                task_map.clear();

                // Close
                // taskflow_.clear();
                // task_map.clear();
                // // Node::Close() , order to process ? reverse of Node::Process() ???
                // for (auto& node : nodes_)
                // {
                //         tf::Task tf_node =
                //             taskflow_.emplace([&]() { node->Close(); })
                //                 .name(std::to_string(node->Id()));

                //         task_map.emplace(std::to_string(node->Id()), tf_node);
                // }

                // for (auto& node : nodes_) {
                //         auto tf_node = task_map.at(std::to_string(node->Id()));
                //         for (auto pred : node->Predecessors()) {
                //                 auto tf_task_pred = task_map.at(pred);
                //                 tf_task_pred.succeed(tf_node); // or
                //                 tf_task_pred.precede(tf_node);
                //         }
                // }

                return 0;
        }

        // TODO: blocked by the implementation of scheduler.
        int Graph::Run()
        {
                RunOnce();
                return Wait();
        }

        // TODO: blocked by the implementation of scheduler.
        int Graph::RunOnce()
        {
                // Process, step2
                fu_ = executor_.run(taskflow_);
                return 0;
        }

        // TODO: blocked by the implementation of scheduler.
        int Graph::Wait()
        {
                // Process, step3
                fu_.wait();

                return 0;
        }

        // TODO: blocked by the implementation of scheduler.
        int Graph::WaitForObservedOutput() { return -1; }

        // TODO: blocked by the implementation of scheduler.
        void Graph::Pause() { return; }

        // TODO: blocked by the implementation of scheduler.
        void Graph::Resume() { return; }

        void Graph::Cancel()
        {
                fu_.cancel();
                fu_.get();
        }

        void Graph::ClearResult()
        {
                for (auto& graph_output : graph_output_streams_)
                {
                        graph_output->Clear();
                }
        }

        int Graph::Finish()
        {
                for (auto& input : graph_input_streams_)
                {
                        input.second->Close();
                }

                for (auto& node : nodes_)
                {
                        node->Finish();
                }

                for (auto& output : graph_output_streams_)
                {
                        output->Close();
                }

                // Process, step4
                taskflow_.clear();

                return 0;
        }

        int Graph::ObserveOutputStream(const std::string& name)
        {
                int idx = cfg_->OutputStreamIndex(name);
                if (idx < 0)
                {
                        SPDLOG_ERROR("invalid GraphOutputStream name {}", name);
                        return -1;
                }
                auto ostream = std::make_unique<GraphOutputStream>();
                ostream->Initialize(name, &output_stream_managers_[idx]);
                graph_output_streams_.push_back(std::move(ostream));

                SPDLOG_DEBUG(
                    "nodes updated by ObserveOutputStream {}:\n{:}", name,
                    fmt::join(nodes_, "\n"));

                return 0;
        }

        int Graph::AddDatumToInputStream(const std::string& name, const Datum& datum)
        {
                return addDatumToInputStream(name, datum);
        }
        int Graph::AddDatumToInputStream(const std::string& name, Datum&& datum)
        {
                return addDatumToInputStream(name, std::move(datum));
        }
        template <typename T>
        int Graph::addDatumToInputStream(const std::string& name, T&& datum)
        {
                auto& stream = graph_input_streams_[name];
                stream->AddDatum(std::forward<T>(datum));
                stream->Propagate();

                return 0;
        }

        int Graph::CloseInputStream(const std::string& name)
        {
                auto& stream = graph_input_streams_[name];
                if (stream->Closed())
                {
                        return 0;
                }
                stream->Close();
                ++num_closed_graph_input_streams_;
                return 0;
        }

        int Graph::CloseAllInputStreams()
        {
                for (auto& stream : graph_input_streams_)
                {
                        stream.second->Close();
                }
                num_closed_graph_input_streams_ = graph_input_streams_.size();
                return 0;
        }

        // TODO: not fully implemented.
        Datum* Graph::OutputSideData(const std::string& name) { return nullptr; }

        void Graph::GraphInputStream::Propagate()
        {
                int nmirrors = manager_->Mirrors().size();
                std::list<Datum>* data = input_.OutputQueue();
                for (int i = 0; i < nmirrors; ++i)
                {
                        auto& m = manager_->Mirrors()[i];
                        SPDLOG_DEBUG(
                            "GraphInputStream {}: propagate data (size:{}) to mirror({})",
                            input_.Name(), data->size(), m);
                        if (i == nmirrors - 1)
                        {
                                m.ish->MoveData(m.id, data);
                        }
                        else
                        {
                                m.ish->AddData(m.id, *data);
                        }
                }
                // clear cache in graph input queue
                data->clear();
        }

        int Graph::AddDatumToInputSideData(const std::string& name, const Datum& datum)
        {
                return addDatumToInputSideData(name, datum);
        }
        // int Graph::AddDatumToInputSideData(const std::string& name, Datum&& datum)
        // {
        //         return addDatumToInputSideData(name, std::move(datum));
        // }

        template <typename T>
        int Graph::addDatumToInputSideData(const std::string& name, const T& datum)
        {
                auto nstreams = cfg_->OutputSideData().size();
                for (int i = 0; i < nstreams; ++i)
                {
                        if (name == output_side_data_[i].Name())
                        {
                                SPDLOG_DEBUG(
                                    "find graph OutputSideDatumImpl, name is: {}",
                                    output_side_data_[i].Name());

                                output_side_data_[i].Set(datum);

                                break;
                        }
                }

                return 0;
        }

        std::optional<GraphConfig> ParsePbtxtToGraphConfig(const std::string& pbtxt)
        {
                std::ifstream fin(pbtxt);
                if (!fin.is_open())
                        return std::nullopt;
                std::stringstream ss;
                ss << fin.rdbuf();
                return ParseStringToGraphConfig(ss.str());
        }

        std::optional<GraphConfig> ParseStringToGraphConfig(const std::string& text)
        {
                GraphConfig config;
                if (!google::protobuf::TextFormat::ParseFromString(text, &config))
                {
                        SPDLOG_ERROR("failed to parse text proto {}", text);
                        return std::nullopt;
                }
                return config;
        }

}   // namespace dni
