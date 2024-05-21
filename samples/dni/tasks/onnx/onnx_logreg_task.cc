
#include <cmath>
#include <cstdint>
#include <unordered_map>
#include <utility>
#include <vector>

#include "dni/framework/datum.h"
#include "dni/framework/framework.h"
#include "dni/tasks/onnx/onnx_task.h"
#include "onnx_defines.h"
#include "spdlog/spdlog.h"

namespace dni {

class OnnxLogRegTask: public OnnxTask {
public:
        OnnxLogRegTask() {}
        ~OnnxLogRegTask() override {}

private:
        virtual void DumpInferenceRet(std::vector<Ort::Value>& inference_ret);
        virtual Datum ParseInferenceResult(std::vector<Ort::Value>& inference_ret);
};

// refer to onnxruntime/test/shared_lib/test_nontensor_types.cc:
// CreateGetVectorOfMapsInt64Float test and
// https://github.com/microsoft/onnxruntime/issues/4620
Datum OnnxLogRegTask::ParseInferenceResult(std::vector<Ort::Value>& inference_ret)
{
        spdlog::info("inference_ret size is: {}", inference_ret.size());

        LogRegInferenceRet ret;

        int output_labels_total = 0;
        for (size_t i = 0; i < loader_->GetOutputNodesNum(); i++)
        {
                if (inference_ret[i].IsTensor())
                {
                        spdlog::info("inference_ret idx:{} is tensor", i);

                        auto outputInfo = inference_ret[i].GetTensorTypeAndShapeInfo();
                        int64_t* int64arr =
                            inference_ret[i].GetTensorMutableData<int64_t>();

                        spdlog::info(
                            "GetElementType is: {}", (int) outputInfo.GetElementType());
                        spdlog::info(
                            "Dimensions of the output is: {}",
                            outputInfo.GetShape().size());

                        std::vector<std::vector<int64_t>> layer1_vec;

                        for (int m = 0; m < outputInfo.GetShape().size(); m++)
                        {
                                auto width = outputInfo.GetShape()[0];

                                std::vector<int64_t> layer2_vec;
                                for (int n = 0; n < width; n++)
                                {
                                        output_labels_total++;

                                        layer2_vec.push_back(*(int64arr + m * width + n));
                                }

                                layer1_vec.push_back(std::move(layer2_vec));
                        }

                        ret.output_labels.push_back(std::move(layer1_vec));

                        spdlog::info("output_labels_total is: {}", output_labels_total);
                }
                else
                {
                        spdlog::info("inference_ret idx:{} is not tensor", i);

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

                                for (unsigned int shapeI = 0;
                                     shapeI < key_outputInfo.GetShape().size();
                                     shapeI++)
                                { keys_total += key_outputInfo.GetShape()[shapeI]; }

                                int64_t* keys_ret =
                                    keys_ort.GetTensorMutableData<int64_t>();

                                // second fetch the values
                                Ort::Value values_ort =
                                    map_out.GetValue(1, default_allocator);

                                auto values_outputInfo =
                                    values_ort.GetTensorTypeAndShapeInfo();
                                int values_total = 0;

                                for (unsigned int shapeI = 0;
                                     shapeI < values_outputInfo.GetShape().size();
                                     shapeI++)
                                {
                                        values_total +=
                                            values_outputInfo.GetShape()[shapeI];
                                }

                                float* values_ret =
                                    values_ort.GetTensorMutableData<float>();

                                if (keys_total != values_total)
                                {
                                        spdlog::info(
                                            "keys_total is not equal to values_total, {} "
                                            "vs {}",
                                            keys_total,
                                            values_total);
                                        return Datum();
                                }

                                std::unordered_map<int64_t, float_t> probabilities;
                                for (int key_idx = 0; key_idx < keys_total; key_idx++)
                                {
                                        probabilities[keys_ret[key_idx]] =
                                            values_ret[key_idx];
                                }

                                ret.output_probabilities.push_back(
                                    std::move(probabilities));
                        }
                }
        }

        return Datum(std::move(ret));
}

void OnnxLogRegTask::DumpInferenceRet(std::vector<Ort::Value>& inference_ret)
{
        std::cout << "inference_ret.size(): " << inference_ret.size() << std::endl;

        int output_labels_total = 0;
        for (size_t i = 0; i < loader_->GetOutputNodesNum(); i++)
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
                                { std::cout << keys_ret[key_idx] << "\t\t\t\t"; }
                                std::cout << std::endl;

                                // second fetch the values
                                Ort::Value values_ort =
                                    map_out.GetValue(1, default_allocator);

                                auto values_outputInfo =
                                    values_ort.GetTensorTypeAndShapeInfo();
                                int values_total = 0;

                                std::cout << "values_outputInfo GetElementType: "
                                          << values_outputInfo.GetElementType() << "\n";
                                std::cout << "values_outputInfo Dimensions of "
                                             "the output: "
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
                                { std::cout << values_ret[val_idx] << "\t\t"; }
                                std::cout << std::endl;
                        }
                }
        }

        std::cout << std::endl;
}

REGISTER(OnnxLogRegTask);

}   // namespace dni
