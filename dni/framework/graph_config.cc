#include "dni/framework/graph_config.h"

#include <fmt/ranges.h>
#include <spdlog/spdlog.h>
#include <string>
#include <vector>

#include "dni/framework/dni.pb.h"
#include "dni/framework/dtype.h"
#include "dni/framework/node_config.h"
#include "dni/framework/utils/names.h"
#include "dni/framework/utils/tags.h"

namespace dni {

        int NodeInfo::Initialize(
            const GraphConfig& graph, const GraphConfig::Node& node, int index)
        {
                ref_.type = NodeType::TASK;
                ref_.index = index;

                // Parse node name.
                std::string cname = utils::CanonicalNodeName(graph, index);
                SPDLOG_DEBUG("node {}: canonical name: {}", node.name(), cname);
                cfg_.SetNodeName(cname);

                // Initialize NodeConfig.
                SPDLOG_DEBUG("node {}: initializing config", cfg_.NodeName());
                if (cfg_.Initialize(node))
                {
                        SPDLOG_ERROR(
                            "node {}: failed to initiate config", cfg_.NodeName());
                        return -1;
                }

                // TODO: Validate InputStreamInfo (additional stream info).
                if (!node.input_stream_info().empty())
                {
                        std::vector<bool> id_used(cfg_.Inputs().size());
                        for (const auto& input_stream_info : node.input_stream_info())
                        {
                                std::string tag;
                                int index = 0;
                                utils::ParseTagIndex(
                                    input_stream_info.tag(), &tag, &index);
                                SPDLOG_DEBUG(
                                    "node {}: InputStreamInfo:{}, tag:{}, index:{}",
                                    cfg_.NodeName(),
                                    input_stream_info.tag(),
                                    tag,
                                    index);
                                int id = cfg_.Inputs().FindByTagIndex(tag, index);
                                if (id < 0)
                                {
                                        SPDLOG_ERROR(
                                            "node {}: cannot find input stream with "
                                            "InputStreamInfo:{}",
                                            cfg_.NodeName(),
                                            input_stream_info.tag());
                                        return -1;
                                }
                                if (id_used[id])
                                {
                                        SPDLOG_ERROR(
                                            "node {}: more than one input stream with "
                                            "InputStreamInfo:{} specified",
                                            cfg_.NodeName(),
                                            input_stream_info.tag());
                                        return -1;
                                }
                                id_used[id] = true;
                        }
                }

                // TODO: Validate input streams.

                // TODO: Validate output streams.

                // TODO: Validate input side data.

                // TODO: Validate output side data.

                predecessors_.clear();

                return 0;
        }

        int ParsedGraphConfig::InitializeNodes()
        {
                nodes_.reserve(proto_.node_size());

                int i = 0;
                for (const auto& node : proto_.node())
                {
                        SPDLOG_DEBUG("initializing NodeInfo of {}", node.name());
                        nodes_.emplace_back();
                        if (nodes_.back().Initialize(proto_, node, i))
                        {
                                SPDLOG_ERROR(
                                    "failed to initialize NodeInfo of {}", node.name());
                                return -1;
                        };
                        SPDLOG_DEBUG("NodeInfo: {}", nodes_.back());

                        ++i;
                }
                return 0;
        }

        // TODO: topological sorting
        int ParsedGraphConfig::topologicalSorting() { return 0; }

        int ParsedGraphConfig::initializeNodeSideData(NodeInfo* node, bool* need_sorting)
        {
                node->input_side_data_base_index = input_side_data_.size();
                for (const std::string& name :
                     node->Config().InputSideData().TagMap()->Names())
                {
                        input_side_data_.emplace_back(
                            EdgeInfo{name, nullptr, node->Node()});
                        auto& edge = input_side_data_.back();
                        auto it = output_side_data_to_index_.find(name);
                        if (it != output_side_data_to_index_.end())
                        {
                                edge.upstream_id = it->second;

                                // this output_side_data may import from graph-level input
                                // side data, not corresponding to a node
                                auto nodeIt = output_side_data_to_node_.find(name);
                                if (nodeIt != output_side_data_to_node_.end())
                                {
                                        node->AddPredecessor(nodeIt->second);
                                }
                        }
                }

                node->output_side_data_base_index = output_side_data_.size();
                for (const std::string& name :
                     node->Config().OutputSideData().TagMap()->Names())
                {
                        output_side_data_.emplace_back(
                            EdgeInfo{name, nullptr, node->Node()});
                        output_side_data_to_index_[name] = output_side_data_.size() - 1;

                        output_side_data_to_node_[name] = node->Config().NodeName();
                }

                return 0;
        }

        int ParsedGraphConfig::InitializeSideData(bool* need_sorting)
        {
                // graph level input side data
                if (proto_.input_side_data().size() != 0)
                {
                        SPDLOG_DEBUG(
                            "graph input side data: {:}", proto_.input_side_data());
                        std::shared_ptr<utils::TagMap> graph_input_sidedata =
                            utils::NewTagMap(proto_.input_side_data());
                        if (!graph_input_sidedata)
                        {
                                SPDLOG_ERROR(
                                    "invalid graph input side data: {}",
                                    proto_.input_side_data());
                                return -1;
                        }

                        auto& names = graph_input_sidedata->Names();
                        for (int i = 0; i < names.size(); ++i)
                        {
                                std::string name = names[i];

                                output_side_data_.emplace_back(
                                    EdgeInfo{name, nullptr, NodeInfo::NodeRef{}});

                                output_side_data_to_index_[name] =
                                    output_side_data_.size() - 1;
                        }
                }

                // node side data
                for (NodeInfo& node : nodes_)
                {
                        if (node.Node().type != NodeInfo::NodeType::TASK)
                        {
                                return -1;
                        }
                        if (initializeNodeSideData(&node, need_sorting))
                        {
                                return -1;
                        }
                }

                return 0;
        }

        int ParsedGraphConfig::initializeNodeStreams(NodeInfo* node, bool* need_sorting)
        {
                // node output
                node->output_stream_base_index = output_streams_.size();
                int idx = 0;
                for (const std::string& name : node->Config().Outputs().TagMap()->Names())
                {
                        SPDLOG_DEBUG(
                            "node {}: output_stream:{}", node->Config().NodeName(), name);
                        output_streams_.emplace_back(
                            EdgeInfo{name, nullptr, node->Node()});
                        output_stream_to_index_[name] = output_streams_.size() - 1;

                        output_stream_to_node_[name] = node->Config().NodeName();

                        idx++;
                }

                // node input
                node->input_stream_base_index = input_streams_.size();
                for (const std::string& name : node->Config().Inputs().TagMap()->Names())
                {
                        SPDLOG_DEBUG(
                            "node {}: input_stream:{}", node->Config().NodeName(), name);
                        input_streams_.emplace_back(
                            EdgeInfo{name, nullptr, node->Node()});
                        auto& edge = input_streams_.back();
                        auto it = output_stream_to_index_.find(name);
                        if (it != output_stream_to_index_.end())
                        {
                                edge.upstream_id = it->second;

                                // this output_stream may import from graph-level input
                                // stream, not corresponding to a node
                                auto nodeIt = output_stream_to_node_.find(name);
                                if (nodeIt != output_stream_to_node_.end())
                                {
                                        node->AddPredecessor(nodeIt->second);
                                }
                        }
                }

                return 0;
        }

        int ParsedGraphConfig::InitializeStreams(bool* need_sorting)
        {
                // graph input
                SPDLOG_DEBUG("graph input streams: {:}", proto_.input_stream());
                std::shared_ptr<utils::TagMap> graph_input_streams =
                    utils::NewTagMap(proto_.input_stream());
                if (!graph_input_streams)
                {
                        SPDLOG_ERROR(
                            "invalid GraphInputStream: {}", proto_.input_stream());
                        return -1;
                }
                auto& names = graph_input_streams->Names();
                for (int i = 0; i < names.size(); ++i)
                {
                        std::string name = names[i];
                        NodeInfo::NodeRef ref{
                            NodeInfo::NodeType::GRAPH_INPUT_STREAM,
                            i + proto_.node_size()};
                        // Add graph input stream to the virtual node.
                        output_streams_.emplace_back(EdgeInfo{name, nullptr, ref});
                        output_stream_to_index_[name] = output_streams_.size() - 1;
                }

                // nodes
                for (NodeInfo& node : nodes_)
                {
                        if (node.Node().type != NodeInfo::NodeType::TASK)
                        {
                                return -1;
                        }
                        if (initializeNodeStreams(&node, need_sorting))
                        {
                                return -1;
                        }

                        SPDLOG_DEBUG(
                            "node {}: predecessors:{:}",
                            node.Config().NodeName(),
                            node.Predecessors());
                }

                SPDLOG_DEBUG("graph output streams: {:}", proto_.output_stream());
                if (!utils::NewTagMap(proto_.output_stream()))
                {
                        SPDLOG_ERROR(
                            "invalid GraphOutputStream: {:}", proto_.output_stream());
                        return -1;
                }
                return 0;
        }

        // Parse protobuf-based GraphConfig and topological sort it into ordered nodes and
        // edges.
        int ParsedGraphConfig::Initialize(GraphConfig config)
        {
                proto_ = std::move(config);

                if (InitializeNodes())
                {
                        return -1;
                }
                SPDLOG_DEBUG("NodeInfos: {:}", nodes_);

                bool need_sorting = false;
                if (InitializeSideData(&need_sorting) || InitializeStreams(&need_sorting))
                {
                        return -1;
                }
                SPDLOG_DEBUG(
                    "input_streams:{:}, output_streams:{:}",
                    input_streams_,
                    output_streams_);

                if (need_sorting)
                {
                        input_streams_.clear();
                        input_side_data_.clear();
                        output_streams_.clear();
                        output_stream_to_index_.clear();
                        output_side_data_.clear();
                        output_side_data_to_index_.clear();
                        output_stream_to_node_.clear();
                        output_side_data_to_node_.clear();
                        if (topologicalSorting())
                        {
                                return -1;
                        }
                        if (InitializeSideData(nullptr) || InitializeStreams(nullptr))
                        {
                                return -1;
                        }
                }

                return 0;
        }

}   // namespace dni
