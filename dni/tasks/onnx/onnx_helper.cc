#include "onnx_helper.h"

#include <memory>
#include <string>
#include <vector>

#include "onnxruntime_cxx_api.h"
#include "spdlog/spdlog.h"

namespace dni {

int parse_for_model_type(const std::string& model_path)
{
        auto env_ = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "onnx-task");

        auto session_options = std::make_unique<Ort::SessionOptions>();
        session_options->SetIntraOpNumThreads(1);
        session_options->SetGraphOptimizationLevel(ORT_ENABLE_BASIC);

        auto session =
            std::make_unique<Ort::Session>(*env_, model_path.c_str(), *session_options);

        // Parse input nodes infos in model
        size_t input_nodes_num;
        std::vector<std::vector<int64_t>> input_node_dims;

        input_nodes_num = session->GetInputCount();
        SPDLOG_DEBUG("Number of inputs = {}", input_nodes_num);

        input_node_dims.resize(input_nodes_num);

        // iterate over all input nodes
        for (size_t i = 0; i < input_nodes_num; i++)
        {
                // Get input node types
                Ort::TypeInfo typeinfo = session->GetInputTypeInfo(i);
                Ort::ConstTensorTypeAndShapeInfo tensor_info =
                    typeinfo.GetTensorTypeAndShapeInfo();

                // Get input shapes/dims/tensor_size
                size_t num_dims;
                num_dims = tensor_info.GetDimensionsCount();
                SPDLOG_DEBUG("Input {} : num_dims = {}", i, num_dims);

                input_node_dims[i].resize(num_dims);
                input_node_dims[i] = tensor_info.GetShape();

                for (size_t j = 0; j < num_dims; j++)
                {
                        SPDLOG_DEBUG(
                            "---Input {} : dim {} = {}", i, j, input_node_dims[i][j]);

                        if (input_node_dims[i][j] == -1)   // dynamic dim
                        {
                                SPDLOG_DEBUG(
                                    "{} is dynamic, in dim-{}-{}", model_path, i, j);
                                return 1;
                        }
                }
        }

        SPDLOG_DEBUG("{} is fixed model", model_path);

        return 0;
}

}   // namespace dni
