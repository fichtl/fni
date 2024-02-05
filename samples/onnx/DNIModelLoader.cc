#include "DNIModelLoader.h"
#include <assert.h>
#include <cmath>
#include <stdlib.h>
#include <iostream>
#include <sstream>


// TODO: err process,

DNIModelLoader::DNIModelLoader(const std::string& model, int threads, GraphOptimizationLevel optimize_level,
                    OrtLoggingLevel logging_level, const std::string& logid) {
    modelPath = model;

    env = new Ort::Env(logging_level, logid.c_str());

    session_options = new Ort::SessionOptions();
    session_options->SetIntraOpNumThreads(threads);
    session_options->SetGraphOptimizationLevel(optimize_level);

    session = new Ort::Session(*env, modelPath.c_str(), *session_options);
}

DNIModelLoader::~DNIModelLoader() {
    if (session) {
        delete session;
    }

    if (env) {
        delete env;
    }

    if (session_options) {
        delete session_options;
    }
}

void DNIModelLoader::Load() {
    // Parse input nodes infos in model
    input_nodes_num = session->GetInputCount();
    std::cout << "Number of inputs = " << input_nodes_num << std::endl;

    input_node_names.resize(input_nodes_num);
    input_node_dims.resize(input_nodes_num);
    input_nodes_tensor_size.resize(input_nodes_num);

    // Parse output nodes infos in model
    output_nodes_num = session->GetOutputCount();
    std::cout << "\n\nNumber of outputs = " << output_nodes_num << std::endl;

    output_node_names.resize(output_nodes_num);
    output_node_dims.resize(output_nodes_num);
    output_nodes_tensor_size.resize(output_nodes_num);

    Ort::AllocatorWithDefaultOptions allocator;

    // iterate over all input nodes
    for (size_t i = 0; i < input_nodes_num; i++) {
        // Get input node names
        char* input_name;
        Ort::AllocatedStringPtr ptr = session->GetInputNameAllocated(i, allocator);
        input_name = ptr.release(); // ptr.get();
        input_node_names[i] = input_name;
        std::cout << "Input " << i << " : name = " << input_node_names[i] << std::endl;

        // Get input node types
        Ort::TypeInfo typeinfo = session->GetInputTypeInfo(i);
        Ort::ConstTensorTypeAndShapeInfo tensor_info = typeinfo.GetTensorTypeAndShapeInfo();

        ONNXTensorElementDataType type;
        type = tensor_info.GetElementType();
        std::cout << "Input " << i << " : type = " << type << std::endl;
        // assert type ???

        // Get input shapes/dims/tensor_size
        size_t num_dims;
        num_dims = tensor_info.GetDimensionsCount();
        std::cout << "Input " << i << " : num_dims = " << num_dims << std::endl;
        input_node_dims[i].resize(num_dims);
        input_node_dims[i] = tensor_info.GetShape();

        for (size_t j = 0; j < num_dims; j++) {
            std::cout << "Input " << i << " : dim " << j << " = " << input_node_dims[i][j] << std::endl;
        }

        input_nodes_tensor_size[i] = tensor_info.GetElementCount();
        std::cout << "Input " << i << " : tensor_size = " << input_nodes_tensor_size[i] << std::endl;

        std::cout << std::endl;
    }

    // iterate over all output nodes
    for (size_t i = 0; i < output_nodes_num; i++) {
        // Get output node names
        char* output_name;
        Ort::AllocatedStringPtr ptr = session->GetOutputNameAllocated(i, allocator);
        output_name = ptr.release(); // ptr.get();
        output_node_names[i] = output_name;
        std::cout << "Output " << i << " : name = " << output_node_names[i] << std::endl;

        // Get output node types
        Ort::TypeInfo typeinfo = session->GetOutputTypeInfo(i);
        Ort::ConstTensorTypeAndShapeInfo tensor_info = typeinfo.GetTensorTypeAndShapeInfo();

        ONNXTensorElementDataType type;
        type = tensor_info.GetElementType();
        std::cout << "Output " << i << " : type = " << type << std::endl;
        // assert type ???

        // Get output shapes/dims
        size_t num_dims;
        num_dims = tensor_info.GetDimensionsCount();
        std::cout << "Output " << i << " : num_dims = " << num_dims << std::endl;
        output_node_dims[i].resize(num_dims);
        output_node_dims[i] = tensor_info.GetShape();

        for (size_t j = 0; j < num_dims; j++) {
            std::cout << "Output " << i << " : dim " << j << " = " << output_node_dims[i][j] << std::endl;
        }

        output_nodes_tensor_size[i] = tensor_info.GetElementCount();
        std::cout << "Output " << i << " : tensor_size = " << output_nodes_tensor_size[i] << std::endl;

        std::cout << std::endl;
    }

}

int DNIModelLoader::Inference(std::vector<std::vector<float>*>& input_data) {

    std::vector<Ort::Value> input_tensors;
    Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

    for (size_t i = 0; i < input_data.size(); i++)
    {
        input_tensors.push_back(Ort::Value::CreateTensor(memory_info, input_data[i]->data(), input_data[i]->size(),
                                                    input_node_dims[i].data(), input_node_dims[i].size()));
    }

    std::vector<Ort::Value> ret;
    try {

        ret = session->Run(Ort::RunOptions{nullptr}, input_node_names.data(), input_tensors.data(), input_tensors.size(),
                    output_node_names.data(), output_node_names.size());

    } catch(const Ort::Exception& exception) {
        // std::cout << "ERROR running model inference: " << exception.what() << std::endl;

        std::ostringstream oss;
        oss << "ERROR running model inference: model is [" << modelPath << "], exception: " << exception.what();
        std::cout << oss.str() << std::endl;

        exit(1);
    }

    /// Get pointer to output tensor float values
    inference_ret.clear();
    for (size_t i = 0; i < output_nodes_num; i++)
    {
        float* floatarr = ret[i].GetTensorMutableData<float>();

        inference_ret.push_back(floatarr);
    }
}

const std::vector<float*>& DNIModelLoader::GetInferenceRet() {
    return this->inference_ret;
}

const std::vector<size_t>& DNIModelLoader::GetOutputNodesTensorSize() {
    return this->output_nodes_tensor_size;
}

size_t DNIModelLoader::GetOutputNodesNum() {
    return this->output_nodes_num;
}

const std::vector<std::vector<int64_t>>& DNIModelLoader::GetOutputNodeDims() {
    return this->output_node_dims;
}


