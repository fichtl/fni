#pragma once

#include "onnx_model_loader.h"

namespace dni {

class ONNXFixedModelLoader: public ONNXModelLoader {
public:
        ONNXFixedModelLoader(
            const std::string& model, int threads, GraphOptimizationLevel optimize_level,
            OrtLoggingLevel logging_level, const std::string& logid)
            : ONNXModelLoader(model, threads, optimize_level, logging_level, logid)
        {}

        ~ONNXFixedModelLoader() {}

        bool Load() override;
        std::optional<std::vector<Ort::Value>> Inference(
            std::vector<std::vector<float>*>& input_data) override;

private:
        std::vector<const char*> input_node_names_;
        std::vector<std::vector<int64_t>> input_node_dims_;
        std::vector<size_t> input_nodes_tensor_size_;

        std::vector<const char*> output_node_names_;
};

}   // namespace dni
