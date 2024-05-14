#pragma once

#include <algorithm>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "dni/framework/dni.pb.h"
#include "dni/framework/dtype.h"
#include "dni/framework/node_config.h"
#include "fmt/format.h"

namespace dni {

        class NodeInfo {
        public:
                enum class NodeType {
                        UNKNOWN = 0,
                        TASK = 1,
                        DATA_GENERATOR = 2,
                        GRAPH_INPUT_STREAM = 3,
                };

                struct NodeRef {
                        NodeRef() = default;
                        NodeRef(NodeType type, int index): type(type), index(index) {}

                        NodeType type = NodeType::UNKNOWN;
                        // Index to node.
                        int index = -1;
                };

                NodeInfo() = default;
                NodeInfo(NodeInfo&&) = default;
                ~NodeInfo() = default;

                int Initialize(
                    const GraphConfig& config, const GraphConfig::Node& node, int index);

                const NodeConfig& Config() const { return cfg_; }

                const NodeRef& Node() const { return ref_; }

                void AddPredecessor(const std::string predecessor_name)
                {
                        predecessors_.emplace(predecessor_name);
                }
                const std::unordered_set<std::string>& Predecessors() const
                {
                        return predecessors_;
                }

                // A node may have more than one streams, so we use base+offset to locate
                // the specific input/output stream/sidedata.
                int input_stream_base_index;
                int output_stream_base_index;
                int input_side_data_base_index;
                int output_side_data_base_index;

                // key: output name, as one output of graph; val: {index
                // in node outputs, index in graph outputs}
                std::unordered_map<std::string, std::pair<int, int>> output_to_graph_;

        private:
                NodeConfig cfg_;
                // Reference to node by (nodetype, index)
                NodeRef ref_;

                // Predecessors of this node, string is node name
                std::unordered_set<std::string> predecessors_;

                friend class fmt::formatter<dni::NodeInfo>;
        };

        struct EdgeInfo {
                // Name of the stream.
                std::string name;

                Dtype* datum_type = nullptr;

                // Reference to the NodeInfo of the node that owns the input stream.
                NodeInfo::NodeRef parent;

                // Index to the upstream output stream's name (std::string).
                int upstream_id = -1;
        };

        class ParsedGraphConfig {
        public:
                int Initialize(GraphConfig config);

                const std::vector<NodeInfo>& Nodes() const { return nodes_; }

                const std::vector<EdgeInfo>& InputStreams() const
                {
                        return input_streams_;
                }
                const std::vector<EdgeInfo>& OutputStreams() const
                {
                        return output_streams_;
                }

                const GraphConfig& Proto() const { return proto_; }

                const std::vector<EdgeInfo>& InputSideData() const
                {
                        return input_side_data_;
                }
                const std::vector<EdgeInfo>& OutputSideData() const
                {
                        return output_side_data_;
                }

                int OutputStreamIndex(const std::string& name) const
                {
                        auto it = output_stream_to_index_.find(name);
                        if (it != output_stream_to_index_.end())
                                return it->second;
                        return -1;
                }

                int OutputSideDataIndex(const std::string& name) const
                {
                        auto it = output_side_data_to_index_.find(name);
                        if (it != output_side_data_to_index_.end())
                                return it->second;
                        return -1;
                }

        private:
                int InitializeNodes();

                int topologicalSorting();

                int initializeNodeStreams(NodeInfo* node, bool* need_sorting);
                int InitializeStreams(bool* need_sorting);

                int initializeNodeSideData(NodeInfo* node, bool* need_sorting);
                int InitializeSideData(bool* need_sorting);

                GraphConfig proto_;

                std::vector<NodeInfo> nodes_;

                std::vector<EdgeInfo> input_streams_;

                std::vector<EdgeInfo> output_streams_;
                // Map from output stream to its index.
                //  key: output stream name.
                //  val: index of the output stream in `output_streams_`.
                std::unordered_map<std::string, int> output_stream_to_index_;
                // Map from output stream to the corresponding node.
                //  key: output stream name;
                //  val: name of the node that the output stream belongs to.
                std::unordered_map<std::string, std::string> output_stream_to_node_;

                std::vector<EdgeInfo> input_side_data_;

                std::vector<EdgeInfo> output_side_data_;
                // Map from name to the index of the global output side data.
                //  key: output side data name.
                //  val: index of the output side data in `output_side_data_`.
                std::unordered_map<std::string, int> output_side_data_to_index_;
                // Map from output side data to the corresponding node.
                //  key: output stream name;
                //  val: name of the node that the output side data belongs to.
                std::unordered_map<std::string, std::string> output_side_data_to_node_;
        };

        std::optional<GraphConfig> ParseTextprotoToGraphConfig(const std::string& input);
        std::optional<GraphConfig> ParseStringToGraphConfig(const std::string& input);
        std::optional<GraphConfig> LoadTextprotoFile(const std::string& fpath);

}   // namespace dni

namespace fmt {

        // TODO:
        template <>
        struct formatter<dni::GraphConfig>: formatter<std::string_view> {
                auto format(const dni::GraphConfig& gc, format_context& ctx) const
                {
                        return format_to(
                            ctx.out(), "{}, node size: {}", gc.type(), gc.node_size());
                }
        };

        template <>
        struct formatter<dni::EdgeInfo>: formatter<std::string_view> {
                auto format(const dni::EdgeInfo& edge, format_context& ctx) const
                {
                        return format_to(ctx.out(), "{}", edge.name);
                }
        };

        template <>
        struct formatter<dni::NodeInfo>: formatter<std::string_view> {
                auto format(const dni::NodeInfo& node, format_context& ctx) const
                {
                        return format_to(
                            ctx.out(), "{{index:{}, type:{}, {}}}", node.ref_.index,
                            fmt::underlying(node.ref_.type), node.cfg_);
                }
        };

}   // namespace fmt
