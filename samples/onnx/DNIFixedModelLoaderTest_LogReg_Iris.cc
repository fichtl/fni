#include <iostream>

#include "DNIFixedModelLoader.h"

/*
[[0 ]],              output_labels_total: 1

value:
0.968525		0.0314749		1.00007e-07


[[2 ]],              output_labels_total: 1

value:
1.37622e-05		0.026101		0.973885

*/

void userPrepareInputData3(std::vector<std::vector<float>*>& input_data)
{
        std::vector<float>* input_tensor = new std::vector<float>();
        input_tensor->push_back((float) 5.1);
        input_tensor->push_back((float) 3.4);
        input_tensor->push_back((float) 1.5);
        input_tensor->push_back((float) 0.2);
        input_data.push_back(input_tensor);
}

void userPrepareInputData4(std::vector<std::vector<float>*>& input_data)
{
        std::vector<float>* input_tensor = new std::vector<float>();
        input_tensor->push_back((float) 6.7);
        input_tensor->push_back((float) 3.2);
        input_tensor->push_back((float) 5.7);
        input_tensor->push_back((float) 2.3);
        input_data.push_back(input_tensor);
}

// refer to onnxruntime/test/shared_lib/test_nontensor_types.cc:
// CreateGetVectorOfMapsInt64Float test and
// https://github.com/microsoft/onnxruntime/issues/4620
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
                        int64_t* int64arr =
                            inference_ret[i].GetTensorMutableData<int64_t>();

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
                                for (int n = 0; n < outputInfo.GetShape()[0]; n++)
                                {
                                        std::cout << *(int64arr + n) << " ";
                                        output_labels_total++;
                                }

                                std::cout << "]" << std::endl;
                        }

                        std::cout << "],              output_labels_total: "
                                  << output_labels_total << std::endl;
                }
                else
                {
                        std::cout << "inference_ret[" << i << "] is not tensor"
                                  << std::endl;
                        std::cout << "inference_ret[i].GetCount(): "
                                  << inference_ret[i].GetCount() << std::endl;
                        std::cout << "inference_ret[i]..GetTypeInfo().GetONNXType(): "
                                  << inference_ret[i].GetTypeInfo().GetONNXType()
                                  << "     "
                                  << inference_ret[i]
                                         .GetTypeInfo()
                                         .GetSequenceTypeInfo()
                                         .GetSequenceElementType()
                                         .GetONNXType()
                                  << std::endl;

                        std::cout << "key: "
                                  << inference_ret[i]
                                         .GetTypeInfo()
                                         .GetSequenceTypeInfo()
                                         .GetSequenceElementType()
                                         .GetMapTypeInfo()
                                         .GetMapKeyType()
                                  << std::endl;
                        std::cout << "val: "
                                  << inference_ret[i]
                                         .GetTypeInfo()
                                         .GetSequenceTypeInfo()
                                         .GetSequenceElementType()
                                         .GetMapTypeInfo()
                                         .GetMapValueType()
                                         .GetONNXType()
                                  << std::endl;

                        Ort::AllocatorWithDefaultOptions default_allocator;

                        // Fetch
                        for (size_t idx = 0; idx < inference_ret[i].GetCount(); ++idx)
                        {
                                Ort::Value map_out = inference_ret[i].GetValue(
                                    static_cast<int>(idx), default_allocator);

                                // fetch the map
                                // first fetch the keys
                                Ort::Value keys_ort =
                                    map_out.GetValue(0, default_allocator);

                                auto key_outputInfo =
                                    keys_ort.GetTensorTypeAndShapeInfo();
                                int keys_total = 0;

                                std::cout << "key_outputInfo GetElementType: "
                                          << key_outputInfo.GetElementType() << "\n";
                                std::cout << "key_outputInfo Dimensions of the output: "
                                          << key_outputInfo.GetShape().size() << "\n";
                                std::cout << "key_outputInfo Shape of the output: ";
                                for (unsigned int shapeI = 0;
                                     shapeI < key_outputInfo.GetShape().size();
                                     shapeI++)
                                {
                                        std::cout << key_outputInfo.GetShape()[shapeI]
                                                  << ", ";
                                        keys_total += key_outputInfo.GetShape()[shapeI];
                                }
                                std::cout << std::endl;

                                int64_t* keys_ret =
                                    keys_ort.GetTensorMutableData<int64_t>();
                                std::cout << "key:   " << std::endl;
                                for (int key_idx = 0; key_idx < keys_total; key_idx++)
                                {
                                        std::cout << keys_ret[key_idx] << "\t\t\t\t";
                                }
                                std::cout << std::endl;

                                // second fetch the values
                                Ort::Value values_ort =
                                    map_out.GetValue(1, default_allocator);

                                auto values_outputInfo =
                                    values_ort.GetTensorTypeAndShapeInfo();
                                int values_total = 0;

                                std::cout << "values_outputInfo GetElementType: "
                                          << values_outputInfo.GetElementType() << "\n";
                                std::cout
                                    << "values_outputInfo Dimensions of the output: "
                                    << values_outputInfo.GetShape().size() << "\n";
                                std::cout << "values_outputInfo Shape of the output: ";
                                for (unsigned int shapeI = 0;
                                     shapeI < values_outputInfo.GetShape().size();
                                     shapeI++)
                                {
                                        std::cout << values_outputInfo.GetShape()[shapeI]
                                                  << ", ";
                                        values_total +=
                                            values_outputInfo.GetShape()[shapeI];
                                }
                                std::cout << std::endl;

                                float* values_ret =
                                    values_ort.GetTensorMutableData<float>();
                                std::cout << "value: " << std::endl;
                                for (int val_idx = 0; val_idx < values_total; val_idx++)
                                {
                                        std::cout << values_ret[val_idx] << "\t\t";
                                }
                                std::cout << std::endl;
                        }
                }
        }

        std::cout << std::endl;
}

int main()
{
        // print model info
        std::string modelPath = "samples/onnx/testdata/logreg_iris_fixed_model.onnx";

        std::shared_ptr<dni::DNIModelLoader> loader =
            std::make_shared<dni::DNIFixedModelLoader>(
                modelPath, 1, ORT_ENABLE_BASIC, ORT_LOGGING_LEVEL_WARNING, "test_c");

        loader->Load();
        // create input1
        std::vector<std::vector<float>*> input_data1;
        userPrepareInputData3(input_data1);   // user call

        // infer1
        std::vector<Ort::Value> ret = loader->Inference(input_data1);
        PrintInferenceRet(loader, ret);

        // create input2
        std::vector<std::vector<float>*> input_data2;
        userPrepareInputData4(input_data2);   // user call

        // infer2
        ret = loader->Inference(input_data2);
        PrintInferenceRet(loader, ret);
}
