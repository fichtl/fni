#include <iostream>

#include "DNIDynamicModelLoader.h"

/*

[
[6.0081 1.9779 -6.9387 ]
[5.27597 1.81978 -6.18197 ]
]


*/

void userPrepareInputData(std::vector<std::vector<float>*>& input_data)
{
        std::vector<float>* input_tensor2 = new std::vector<float>();

        input_tensor2->push_back((float) 5.1);
        input_tensor2->push_back((float) 3.5);
        input_tensor2->push_back((float) 1.4);
        input_tensor2->push_back((float) 0.2);

        input_tensor2->push_back((float) 4.9);
        input_tensor2->push_back((float) 3.0);
        input_tensor2->push_back((float) 1.4);
        input_tensor2->push_back((float) 0.2);

        input_data.push_back(input_tensor2);
}

// refer to onnxruntime/test/shared_lib/test_nontensor_types.cc:
// CreateGetVectorOfMapsInt64Float test
void PrintInferenceRet(
    std::shared_ptr<dni::DNIModelLoader> loader, std::vector<Ort::Value>& inference_ret)
{
        std::cout << "inference_ret.size(): " << inference_ret.size() << std::endl;

        int output_labels_total = 0;
        for (size_t i = 0; i < loader->GetOutputNodesNum(); i++)
        {
                if (inference_ret[i].IsTensor())
                {
                        auto outputInfo = inference_ret[i].GetTensorTypeAndShapeInfo();
                        float* floatarr = inference_ret[i].GetTensorMutableData<float>();

                        std::cout << "GetElementType: " << outputInfo.GetElementType()
                                  << "\n";
                        std::cout << "Dimensions of the output: "
                                  << outputInfo.GetShape().size() << "\n";
                        std::cout << "Shape of the output: ";
                        for (unsigned int shapeI = 0;
                             shapeI < outputInfo.GetShape().size();
                             shapeI++)
                                std::cout << outputInfo.GetShape()[shapeI] << ", ";

                        std::cout << std::endl << "[" << std::endl;
                        for (int m = 0; m < outputInfo.GetShape().size(); m++)
                        {
                                std::cout << "[";
                                for (int n = 0; n < outputInfo.GetShape()[1]; n++)
                                {
                                        std::cout << *(floatarr +
                                                       m * outputInfo.GetShape()[1] + n)
                                                  << " ";
                                }
                                std::cout << "]" << std::endl;
                        }

                        std::cout << "]" << std::endl;
                }
        }

        std::cout << std::endl;
}

int main()
{
        // print model info
        std::string modelPath =
            "/home/zhouxu/works/2024/50_dni/0325/dni/samples/onnx/bpnet_iris.onnx";

        std::shared_ptr<dni::DNIModelLoader> loader =
            std::make_shared<dni::DNIDynamicModelLoader>(
                modelPath, 1, ORT_ENABLE_BASIC, ORT_LOGGING_LEVEL_WARNING, "test_c");

        loader->Load();
        // create input1
        std::vector<std::vector<float>*> input_data1;
        userPrepareInputData(input_data1);   // user call

        // infer1
        std::vector<Ort::Value> ret = loader->Inference(input_data1);
        PrintInferenceRet(loader, ret);
}
