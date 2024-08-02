
#include <cmath>
#include <cstdint>
#include <unordered_map>
#include <utility>
#include <vector>

#include "dni/framework/datum.h"
#include "dni/framework/framework.h"
#include "dni/tasks/onnx/onnx_task.h"
// #include "onnx_defines.h"
#include "spdlog/spdlog.h"

namespace dni {

class OnnxSNDingMFRTask: public OnnxTask {
public:
        OnnxSNDingMFRTask() {}
        ~OnnxSNDingMFRTask() override {}

private:
        virtual void DumpInferenceRet(std::vector<Ort::Value>& inference_ret);
        virtual Datum ParseInferenceResult(std::vector<Ort::Value>& inference_ret);
};

Datum OnnxSNDingMFRTask::ParseInferenceResult(std::vector<Ort::Value>& inference_ret)
{
        spdlog::info("inference_ret size is: {}", inference_ret.size());

        std::vector<std::vector<std::vector<float_t>>> ret;

        for (size_t i = 0; i < inference_ret.size(); i++)
        {
                if (!inference_ret[i].IsTensor())
                {
                        spdlog::info("inference_ret idx {} is not tensor", i);
                        continue;
                }

                std::vector<std::vector<float_t>> first_layer_vec;

                auto outputInfo = inference_ret[i].GetTensorTypeAndShapeInfo();
                float* floatarr = inference_ret[i].GetTensorMutableData<float>();

                spdlog::info("GetElementType is: {}", (int) outputInfo.GetElementType());
                spdlog::info(
                    "Dimensions of the output is: {}", outputInfo.GetShape().size());

                if (outputInfo.GetShape().size() != 2)
                {
                        spdlog::error(
                            "output dims is not 2, it is {}",
                            outputInfo.GetShape().size());

                        return Datum();
                }

                spdlog::info(
                    "out_shape, {}, {}",
                    outputInfo.GetShape()[0],
                    outputInfo.GetShape()[1]);

                for (int m = 0; m < outputInfo.GetShape()[0]; m++)
                {
                        std::vector<float_t> second_layer_vec;

                        for (int n = 0; n < outputInfo.GetShape()[1]; n++)
                        {
                                second_layer_vec.emplace_back(
                                    *(floatarr + m * outputInfo.GetShape()[1] + n));
                        }
                        first_layer_vec.emplace_back(std::move(second_layer_vec));
                }

                ret.emplace_back(std::move(first_layer_vec));
        }

        return Datum(std::move(ret));
}

void OnnxSNDingMFRTask::DumpInferenceRet(std::vector<Ort::Value>& inference_ret)
{
        std::cout << "----------- Begin OnnxSNDingMFRTask::DumpInferenceRet() -----------"
                  << std::endl;
        std::cout << inference_ret.size() << std::endl;

        for (size_t i = 0; i < inference_ret.size(); i++)
        {
                if (!inference_ret[i].IsTensor())
                {
                        spdlog::info("inference_ret idx {} is not tensor", i);
                        continue;
                }

                auto outputInfo = inference_ret[i].GetTensorTypeAndShapeInfo();
                float* floatarr = inference_ret[i].GetTensorMutableData<float>();

                if (outputInfo.GetShape().size() != 2)
                {
                        spdlog::error(
                            "output dims is not 2, it is {}",
                            outputInfo.GetShape().size());

                        return;
                }

                std::cout << "GetElementType: " << outputInfo.GetElementType() << "\n";
                std::cout << "Dimensions of the output: " << outputInfo.GetShape().size()
                          << "\n";
                std::cout << "Shape of the output: ";
                for (auto shapeI = 0; shapeI < outputInfo.GetShape().size(); shapeI++)
                        std::cout << outputInfo.GetShape()[shapeI] << ", ";

                std::cout << std::endl << "[" << std::endl;
                for (int m = 0; m < outputInfo.GetShape()[0]; m++)
                {
                        std::cout << "[";
                        for (int n = 0; n < outputInfo.GetShape()[1]; n++)
                        {
                                std::cout
                                    << *(floatarr + m * outputInfo.GetShape()[1] + n)
                                    << " ";
                        }
                        std::cout << "]," << std::endl;
                }

                std::cout << "]" << std::endl;
        }

        std::cout << "----------- End OnnxSNDingMFRTask::DumpInferenceRet() -----------"
                  << std::endl
                  << std::endl;
}

REGISTER(OnnxSNDingMFRTask);

}   // namespace dni
