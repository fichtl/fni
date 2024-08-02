
#include <cmath>
#include <utility>
#include <vector>

#include "dni/framework/datum.h"
#include "dni/framework/framework.h"
#include "dni/tasks/onnx/onnx_task.h"
#include "spdlog/spdlog.h"

namespace dni {

class OnnxTwoDimesionTask: public OnnxTask {
public:
        OnnxTwoDimesionTask() {}
        ~OnnxTwoDimesionTask() override {}

private:
        virtual Datum ParseInferenceResult(std::vector<Ort::Value>& inference_ret);
        virtual void DumpInferenceRet(std::vector<Ort::Value>& inference_ret);
};

Datum OnnxTwoDimesionTask::ParseInferenceResult(std::vector<Ort::Value>& inference_ret)
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

void OnnxTwoDimesionTask::DumpInferenceRet(std::vector<Ort::Value>& inference_ret)
{
        std::cout << inference_ret.size() << std::endl;

        for (size_t i = 0; i < inference_ret.size(); i++)
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

REGISTER(OnnxTwoDimesionTask);

}   // namespace dni
