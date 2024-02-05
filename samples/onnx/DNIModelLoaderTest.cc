#include "DNIModelLoader.h"
#include <iostream>

void userPrepareInputData(std::vector<std::vector<float>*>& input_data) {
    std::vector<float>* input_tensor_values1 = new std::vector<float>();
    input_tensor_values1->push_back((float)1.00);
    input_tensor_values1->push_back((float)2.00);
    input_tensor_values1->push_back((float)3.00);
    input_tensor_values1->push_back((float)4.00);
    input_data.push_back(input_tensor_values1);

    std::vector<float>* input_tensor_values2 = new std::vector<float>();
    input_tensor_values2->push_back((float)5.00);
    input_tensor_values2->push_back((float)6.00);
    input_tensor_values2->push_back((float)7.00);
    input_tensor_values2->push_back((float)8.00);
    input_data.push_back(input_tensor_values2);

    std::vector<float>* input_tensor_values3 = new std::vector<float>();
    input_tensor_values3->push_back((float)11.00);
    input_tensor_values3->push_back((float)12.00);
    input_tensor_values3->push_back((float)13.00);
    input_tensor_values3->push_back((float)14.00);
    input_data.push_back(input_tensor_values3);
}

void userPrepareInputData2(std::vector<std::vector<float>*>& input_data) {
    std::vector<float>* input_tensor_values1 = new std::vector<float>();
    input_tensor_values1->push_back((float)4.00);
    input_tensor_values1->push_back((float)3.00);
    input_tensor_values1->push_back((float)2.00);
    input_tensor_values1->push_back((float)1.00);
    input_data.push_back(input_tensor_values1);

    std::vector<float>* input_tensor_values2 = new std::vector<float>();
    input_tensor_values2->push_back((float)8.00);
    input_tensor_values2->push_back((float)7.00);
    input_tensor_values2->push_back((float)6.00);
    input_tensor_values2->push_back((float)5.00);
    input_data.push_back(input_tensor_values2);

    std::vector<float>* input_tensor_values3 = new std::vector<float>();
    input_tensor_values3->push_back((float)111.00);
    input_tensor_values3->push_back((float)112.00);
    input_tensor_values3->push_back((float)113.00);
    input_tensor_values3->push_back((float)114.00);
    input_data.push_back(input_tensor_values3);
}

void print_inference_ret(const std::vector<float *>& inference_ret, size_t output_nodes_num, const std::vector<size_t>& output_nodes_tensor_size) {
    std::cout << inference_ret.size() << std::endl;

    for (size_t i = 0; i < output_nodes_num; i++)
    {
        float* floatarr = inference_ret[i];

        for (int j = 0; j < output_nodes_tensor_size[i]; j++) {
            std::cout << "Output [" << i << "], tensor value [" << j << "] = " << floatarr[j] << std::endl;
        }
    }

    std::cout << std::endl;
}

int main() {
    std::string modelPath = "/home/zhouxu/works/2024/50_dni/dni/samples/onnx/model19.onnx";

    DNIModelLoader* loader = new DNIModelLoader(modelPath, 1, ORT_ENABLE_BASIC,
                            ORT_LOGGING_LEVEL_WARNING, "test_cxx");

    loader->Load();

    // 1
    // create input tensor object from data values
    std::vector<std::vector<float>*> input_data;
    userPrepareInputData(input_data);// user call

    loader->Inference(input_data);
    print_inference_ret(loader->GetInferenceRet(), loader->GetOutputNodesNum(), loader->GetOutputNodesTensorSize());
    std::cout << "main done\n\n" << std::endl;


    // 2
    // create input tensor object from data values
    std::vector<std::vector<float>*> input_data2;
    userPrepareInputData2(input_data2);// user call

    loader->Inference(input_data2);
    print_inference_ret(loader->GetInferenceRet(), loader->GetOutputNodesNum(), loader->GetOutputNodesTensorSize());
    std::cout << "main done--2" << std::endl;

    delete loader;
}



// int main() {
//     std::string modelPath = "/home/zhouxu/works/2024/50_dni/dni/samples/onnx/model19.onnx";

//     DNIModelLoader* loader = new DNIModelLoader(modelPath, 1, ORT_ENABLE_BASIC,
//                             ORT_LOGGING_LEVEL_WARNING, "test_cxx");

//     loader->Load();

//     // 1
//     // create input tensor object from data values
//     std::vector<std::vector<float>*> input_data;
//     userPrepareInputData(input_data);// user call

//     loader->Inference(input_data);
//     print_inference_ret(loader->GetInferenceRet(), loader->GetOutputNodesNum(), loader->GetOutputNodesTensorSize());
//     std::cout << "main done\n\n" << std::endl;


//     // 2
//     // create input tensor object from data values
//     std::vector<std::vector<float>*> input_data2;
//     userPrepareInputData2(input_data2);// user call

//     loader->Inference(input_data2);
//     print_inference_ret(loader->GetInferenceRet(), loader->GetOutputNodesNum(), loader->GetOutputNodesTensorSize());
//     std::cout << "main done--2" << std::endl;

//     delete loader;
// }

// int main() {
//     std::string modelPath = "/home/zhouxu/works/2024/50_dni/dni/samples/onnx/model19.onnx";

//     // 1
//     Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "test_cxx");

//     Ort::SessionOptions session_options{};
//     session_options.SetIntraOpNumThreads(1);
//     session_options.SetGraphOptimizationLevel(ORT_ENABLE_BASIC);

//     DNIModelLoader* loader = new DNIModelLoader(env, modelPath, session_options);

//     loader->Load();

//     // create input tensor object from data values
//     std::vector<std::vector<float>*> input_data;
//     userPrepareInputData(input_data);// user call

//     loader->Inference(input_data);

//     //delete loader;

//     std::cout << "main done" << std::endl;


//     // 2
// #if 0
//     DNIModelLoader* loader2 = new DNIModelLoader(modelPath, 1, ORT_ENABLE_BASIC,
//                                 ORT_LOGGING_LEVEL_WARNING, "test_cxx");
// #endif

//     DNIModelLoader* loader2 = loader;

//     loader2->Load();

//     // create input tensor object from data values
//     std::vector<std::vector<float>*> input_data2;
//     userPrepareInputData2(input_data2);// user call

//     loader2->Inference(input_data2);

//     delete loader2;

//     std::cout << "main done--2" << std::endl;
// }

