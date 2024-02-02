#pragma once

#include <atomic>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "dni/framework/datum.h"
#include "dni/framework/dni.pb.h"
#include "dni/framework/graph_config.h"
#include "dni/framework/graph_output_stream.h"
#include "dni/framework/input_stream_manager.h"
#include "dni/framework/node.h"
#include "dni/framework/output_side_data.h"
#include "dni/framework/output_stream_manager.h"
#include "taskflow/taskflow.hpp"

namespace dni {

        class Graph {
        public:
                Graph() {}
                Graph(const Graph&) = delete;
                Graph& operator=(const Graph&) = delete;
                Graph(GraphConfig config);
                virtual ~Graph() {}

                int Initialize(GraphConfig config);
                int Initialize(
                    GraphConfig config, const std::map<std::string, Datum>& side_data);

                int PrepareForRun();

                virtual int Run();

                int RunOnce();

                int Wait();

                int WaitForObservedOutput();

                bool HasError() const { return has_error_; }

                void Pause();

                void Resume();

                void Cancel();

                int Finish();

                int AddDatumToInputStream(const std::string& stream_name, Datum&& datum);
                int AddDatumToInputStream(
                    const std::string& stream_name, const Datum& datum);

                int CloseInputStream(const std::string& stream_name);

                int CloseAllInputStreams();

                Datum* OutputSideData(const std::string& datum_name);

                int ObserveOutputStream(const std::string& name);

                // TODO: remove this after re-inspecting the implementation of
                // ParsedGraphConfig::InitializeStreams and Node::initializeOutputStream.
                // It's supposed to be included by the last node's output streams
                // naturally.
                // InputStreamHandler* GetGraphOutputStreamHandler(int index);

                template <typename T>
                T GetResult(const std::string& graph_output_name) const;

                void ClearResult();

        private:
                class GraphInputStream {
                public:
                        explicit GraphInputStream(OutputStreamManager* manager)
                            : manager_(manager)
                        {
                                input_.SetSpec(manager_->Spec());
                        }

                        void AddDatum(const Datum& datum) { input_.AddDatum(datum); }

                        void Propagate()
                        {
                                for (auto& m : manager_->Mirrors())
                                {
                                        SPDLOG_DEBUG("propagate inputs to mirror {}", m);
                                        m.ish->AddData(m.id, *input_.OutputQueue());
                                }
                        }

                        void Close() { closed_ = true; }
                        bool Closed() { return closed_; }

                private:
                        OutputStreamManager* manager_;
                        OutputStreamImpl input_;

                        bool closed_;
                };

                int InitializeStreams();
                int InitializeNodes();

                template <typename T>
                int addDatumToInputStream(const std::string& name, T&& datum);

                bool initialized_ = false;

                std::atomic<bool> has_error_ = false;

                std::unique_ptr<ParsedGraphConfig> cfg_;

                std::unordered_map<std::string, std::unique_ptr<GraphInputStream>>
                    graph_input_streams_;
                std::unordered_map<std::string, int> graph_input_stream_to_node_;
                std::atomic<unsigned int> num_closed_graph_input_streams_;

                std::vector<std::shared_ptr<GraphOutputStream>> graph_output_streams_;

                std::unique_ptr<InputStreamManager[]> input_stream_managers_;
                std::unordered_map<InputStreamManager*, int> input_stream_manager_lookup_;

                std::unique_ptr<OutputStreamManager[]> output_stream_managers_;
                std::unordered_map<OutputStreamManager*, int>
                    output_stream_manager_lookup_;

                std::unique_ptr<OutputSideDatumImpl[]> output_side_data_;

                std::vector<std::unique_ptr<Node>> nodes_;

                std::map<std::string, Datum> current_input_side_data_;

                // TODO: Scheduler
                tf::Executor executor_;
                tf::Taskflow taskflow_;
                tf::Future<void> fu_;
        };

        template <typename T>
        inline T Graph::GetResult(const std::string& name) const
        {
                for (auto graph_output_stream : graph_output_streams_)
                {
                        if (name == graph_output_stream->GetInputStreamManager()->Name())
                        {
                                return graph_output_stream->Value<T>();
                        }
                }
                return T();
        }

        std::optional<GraphConfig> ParseStringToGraphConfig(const std::string& input);
        std::optional<GraphConfig> ParsePbtxtToGraphConfig(const std::string& pbtxt);

}   // namespace dni
