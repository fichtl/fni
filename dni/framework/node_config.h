#pragma once

#include <memory>
#include <string>

#include "dni/framework/dni.pb.h"
#include "dni/framework/dtype.h"
#include "fmt/format.h"
#include "fmt/ranges.h"

namespace dni {

        class NodeConfig {
        public:
                int Initialize(const GraphConfig::Node& node);

                const GraphConfig::Node& Proto() const { return *proto_; }

                // Node name should be assigned by ParsedGraphConfig since there might be
                // name conflicts.
                void SetNodeName(const std::string& name) { node_name_ = name; }
                const std::string& NodeName() const { return node_name_; }

                DtypeSet& Inputs() { return *inputs_; }
                const DtypeSet& Inputs() const { return *inputs_; }

                DtypeSet& Outputs() { return *outputs_; }
                const DtypeSet& Outputs() const { return *outputs_; }

                DtypeSet& InputSideData() { return *input_side_data_; }
                const DtypeSet& InputSideData() const { return *input_side_data_; }

                DtypeSet& OutputSideData() { return *output_side_data_; }
                const DtypeSet& OutputSideData() const { return *output_side_data_; }

        private:
                // Canonical name of the node.
                std::string node_name_;
                const GraphConfig::Node* proto_ = nullptr;

                std::unique_ptr<DtypeSet> inputs_;
                std::unique_ptr<DtypeSet> outputs_;
                std::unique_ptr<DtypeSet> input_side_data_;
                std::unique_ptr<DtypeSet> output_side_data_;

                friend class Node;
                friend class fmt::formatter<dni::NodeConfig>;
        };

}   // namespace dni

namespace fmt {

        template <>
        struct formatter<dni::NodeConfig>: formatter<std::string_view> {
                auto format(const dni::NodeConfig& node, format_context& ctx) const
                {
                        return format_to(
                            ctx.out(),
                            "name:{}, inputs:{:}, outputs:{:}, inputsidedata:{:}, "
                            "outputsidedata:{:}",
                            node.node_name_, node.inputs_->TagMap()->Names(),
                            node.outputs_->TagMap()->Names(),
                            node.input_side_data_->TagMap()->Names(),
                            node.output_side_data_->TagMap()->Names());
                }
        };

}   // namespace fmt
