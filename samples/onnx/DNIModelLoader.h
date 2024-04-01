#pragma once

#include <memory>
#include <onnx/onnxruntime_cxx_api.h>
#include <string>
#include <vector>

namespace dni {

        class DNIModelLoader {
        public:
                DNIModelLoader(
                    const std::string& model, int threads,
                    GraphOptimizationLevel optimize_level, OrtLoggingLevel logging_level,
                    const std::string& logid);

                ~DNIModelLoader();

                virtual void Load() = 0;
                virtual std::vector<Ort::Value> Inference(
                    std::vector<std::vector<float>*>& input_data) = 0;

                size_t GetOutputNodesNum() { return this->output_nodes_num_; }

        protected:
                std::string modelPath_;
                std::unique_ptr<Ort::Session> session_;
                std::unique_ptr<Ort::Env> env_;
                std::unique_ptr<Ort::SessionOptions> session_options_;

                size_t input_nodes_num_;

                size_t output_nodes_num_;
        };

}   // namespace dni