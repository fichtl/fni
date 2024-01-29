#pragma once

#include <string>
#include <vector>

#include "dni/framework/datum_type.h"
#include "dni/framework/dni.pb.h"
#include "dni/framework/node_config.h"

namespace dni {

        class ValidatedGraphConfig;

        class NodeInfo {
        public:
                enum class NodeType {
                        UNKNOWN = 0,
                        TASK = 1,
                        DATA_GENERATOR = 2,
                };

                struct NodeRef {
                        NodeRef() = default;
                        NodeRef(NodeType type, int index): type(type), index(index) {}

                        NodeType type = NodeType::UNKNOWN;

                        int index = -1;
                };

                NodeInfo() = default;
                ~NodeInfo() = default;

                int Initialize(
                    const ValidatedGraphConfig& config, const GraphConfig::Node& node,
                    int index);

                const NodeConfig& Config() const { return cfg_; }

                const NodeRef& Node() const { return node_; }

        private:
                NodeConfig cfg_;
                NodeRef node_;
        };

        struct EdgeInfo {
                std::string name;

                DatumType* datum_type = nullptr;

                NodeInfo::NodeRef parent;
        };

        class ValidatedGraphConfig {
        public:
                int Initialize(GraphConfig config);
                int Initialize(std::string& graph_type);

                const std::vector<EdgeInfo>& InputStreams() { return input_streams_; }
                const std::vector<EdgeInfo>& OutputStreams() { return output_streams_; }

                const GraphConfig& Config() const { return config_; }

        private:
                GraphConfig config_;

                std::vector<NodeInfo> nodes_;

                std::vector<EdgeInfo> input_streams_;
                std::vector<EdgeInfo> output_streams_;
                std::vector<EdgeInfo> input_side_data_;
                std::vector<EdgeInfo> output_side_data_;
        };

}   // namespace dni
