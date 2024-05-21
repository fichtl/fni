#pragma once

#include <memory>

#include "dni/framework/framework.h"
#include "dni/tasks/onnx/onnx.h"
#include "dni/tasks/onnx/onnx_task.pb.h"

namespace dni {

class OnnxTask: public TaskBase {
public:
        OnnxTask() {}
        ~OnnxTask() override {}

        int Open(TaskContext* ctx) override;

        int Process(TaskContext* ctx) override;

        int Close(TaskContext* ctx) override;

protected:
        std::shared_ptr<ONNXModelLoader> loader_;

private:
        virtual Datum ParseInferenceResult(std::vector<Ort::Value>& inference_ret) = 0;
        virtual void DumpInferenceRet(std::vector<Ort::Value>& inference_ret) = 0;

        std::string name_;

        OnnxTaskOptions options_;
        std::string model_path_;
        int model_type_;
};

}   // namespace dni
