#ifndef _DNIMODELLOADER_H
#define _DNIMODELLOADER_H

#include <onnx/onnxruntime_cxx_api.h>
#include <string>
#include <vector>

class DNIModelLoader {
public:
    DNIModelLoader(const std::string& model, int threads, GraphOptimizationLevel optimize_level,
                    OrtLoggingLevel logging_level, const std::string& logid);

    ~DNIModelLoader();

    void Load();
    int Inference(std::vector<std::vector<float>*>& input_data);

    const std::vector<float*>& GetInferenceRet();
    const std::vector<size_t>& GetOutputNodesTensorSize();
    const std::vector<std::vector<int64_t>>& GetOutputNodeDims();
    size_t GetOutputNodesNum();


private:

    std::string modelPath;
    Ort::Session* session;
    Ort::Env* env;
    Ort::SessionOptions* session_options;

    size_t input_nodes_num;
    std::vector<const char*> input_node_names;
    std::vector<std::vector<int64_t>> input_node_dims;
    std::vector<size_t> input_nodes_tensor_size;

    size_t output_nodes_num;
    std::vector<const char*> output_node_names;
    std::vector<std::vector<int64_t>> output_node_dims;
    std::vector<size_t> output_nodes_tensor_size;

    std::vector<float*> inference_ret;
};

#endif