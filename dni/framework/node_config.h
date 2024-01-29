#pragma once

#include <memory>
#include <string>

#include "dni/framework/datum_type.h"
#include "dni/framework/dni.pb.h"

namespace dni {

        class NodeConfig {
        public:
                int Initialize(GraphConfig::Node& node);

                DatumTypeSet& Inputs() { return *inputs_; }

                DatumTypeSet& Outputs() { return *outputs_; }

                DatumTypeSet& InputSideData() { return *input_side_data_; }

                DatumTypeSet& OutputSideData() { return *output_side_data_; }

        private:
                std::string node_name_;
                const GraphConfig::Node* node_config_ = nullptr;

                std::unique_ptr<DatumTypeSet> inputs_;
                std::unique_ptr<DatumTypeSet> outputs_;
                std::unique_ptr<DatumTypeSet> input_side_data_;
                std::unique_ptr<DatumTypeSet> output_side_data_;

                friend class Node;
        };

}   // namespace dni
