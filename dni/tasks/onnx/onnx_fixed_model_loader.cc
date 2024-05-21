#include "onnx_fixed_model_loader.h"

#include <optional>
#include <vector>

#include "spdlog/spdlog.h"

namespace dni {

bool ONNXFixedModelLoader::Load()
{
        // Parse input nodes infos in model
        input_nodes_num_ = session_->GetInputCount();
        SPDLOG_INFO("Number of inputs = {}", input_nodes_num_);

        input_node_names_.resize(input_nodes_num_);
        input_node_dims_.resize(input_nodes_num_);
        input_nodes_tensor_size_.resize(input_nodes_num_);

        // Parse output nodes infos in model
        output_nodes_num_ = session_->GetOutputCount();
        SPDLOG_INFO("Number of outputs = {}", output_nodes_num_);

        output_node_names_.resize(output_nodes_num_);

        Ort::AllocatorWithDefaultOptions allocator;

        // iterate over all input nodes
        for (size_t i = 0; i < input_nodes_num_; i++)
        {
                // Get input node names
                char* input_name;
                Ort::AllocatedStringPtr ptr =
                    session_->GetInputNameAllocated(i, allocator);
                input_name = ptr.release();   // ptr.get();
                input_node_names_[i] = input_name;
                SPDLOG_DEBUG("Input {} : name = {}", i, input_node_names_[i]);

                // Get input node types
                Ort::TypeInfo typeinfo = session_->GetInputTypeInfo(i);
                Ort::ConstTensorTypeAndShapeInfo tensor_info =
                    typeinfo.GetTensorTypeAndShapeInfo();

                ONNXTensorElementDataType type;
                type = tensor_info.GetElementType();
                SPDLOG_DEBUG("Input {} : type = {}", i, (int) type);

                // Get input shapes/dims/tensor_size
                size_t num_dims;
                num_dims = tensor_info.GetDimensionsCount();
                SPDLOG_INFO("Input {} : num_dims = {}", i, num_dims);

                input_node_dims_[i].resize(num_dims);
                input_node_dims_[i] = tensor_info.GetShape();

                for (size_t j = 0; j < num_dims; j++)
                {
                        SPDLOG_DEBUG(
                            "---Input {} : dim {} = {}", i, j, input_node_dims_[i][j]);
                }

                input_nodes_tensor_size_[i] = tensor_info.GetElementCount();

                SPDLOG_DEBUG(
                    "Input {} : tensor_size = {}", i, input_nodes_tensor_size_[i]);
        }

        // iterate over all output nodes
        for (size_t i = 0; i < output_nodes_num_; i++)
        {
                // Get output node names
                char* output_name;
                Ort::AllocatedStringPtr ptr =
                    session_->GetOutputNameAllocated(i, allocator);
                output_name = ptr.release();   // ptr.get();
                output_node_names_[i] = output_name;

                SPDLOG_DEBUG("Output {} : name = {}", i, output_node_names_[i]);
        }

        return true;
}

std::optional<std::vector<Ort::Value>> ONNXFixedModelLoader::Inference(
    std::vector<std::vector<float>*>& input_data)
{
        std::vector<Ort::Value> input_tensors;
        Ort::MemoryInfo memory_info =
            Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

        for (size_t i = 0; i < input_data.size(); i++)
        {
                input_tensors.push_back(Ort::Value::CreateTensor(
                    memory_info, input_data[i]->data(), input_data[i]->size(),
                    input_node_dims_[i].data(), input_node_dims_[i].size()));
        }

        try
        {
                return std::move(session_->Run(
                    Ort::RunOptions{nullptr}, input_node_names_.data(),
                    input_tensors.data(), input_tensors.size(), output_node_names_.data(),
                    output_node_names_.size()));
        }
        catch (const Ort::Exception& exception)
        {
                SPDLOG_CRITICAL(
                    "onnx model inference failed, ERROR running model inference: model "
                    "is [{}], exception: {}",
                    modelPath_, exception.what());

                std::vector<Ort::Value> ret;

                return std::nullopt;
        }
}

}   // namespace dni
