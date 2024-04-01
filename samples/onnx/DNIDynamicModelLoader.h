#pragma once

#include "DNIModelLoader.h"

namespace dni {

        class DNIDynamicModelLoader: public DNIModelLoader {
        public:
                DNIDynamicModelLoader(
                    const std::string& model, int threads,
                    GraphOptimizationLevel optimize_level, OrtLoggingLevel logging_level,
                    const std::string& logid)
                    : DNIModelLoader(model, threads, optimize_level, logging_level, logid)
                {}

                ~DNIDynamicModelLoader() {}

                void Load() override;
                std::vector<Ort::Value> Inference(
                    std::vector<std::vector<float>*>& input_data) override;

        private:
                std::vector<const char*> input_node_names_;
                std::vector<std::vector<int64_t>> input_node_dims_;

                std::vector<const char*> output_node_names_;

                // vector size is same as the number of input nodes, each item in
                // inputs_dynamic_status represents one node dynamic or not
                std::vector<bool> inputs_dynamic_status_;

                // vector size is same as the number of input nodes, each node can only
                // contain one dynamic dim, record the index of the inputs of the node, if
                // inputs of the node is fixed, ignore the item of this vector
                std::vector<int> inputs_dynamic_index_;

                // notice, each item is dims accumulate without dynamic dim, if shape is
                // [2,3,4], item in vector is 24; if shape is [2,3,-1], is 6
                std::vector<std::size_t> inputs_size_in_1D_;
        };

}   // namespace dni