#include "dni/tasks/onnx/onnx_task.h"

#include "spdlog/spdlog.h"

namespace dni {

int OnnxTask::Open(TaskContext* ctx)
{
        name_ += "(" + ctx->Name() + ")";
        SPDLOG_DEBUG("{}: open task ...", name_);

        options_ = ctx->Options<OnnxTaskOptions>();
        model_path_ = options_.model_path();

        SPDLOG_DEBUG("{}: model_path: {}", name_, model_path_);
        model_type_ = parse_for_model_type(model_path_);

        switch (model_type_)
        {
        case 0: {
                loader_ = std::make_shared<ONNXFixedModelLoader>(
                    model_path_, 1, ORT_ENABLE_BASIC, ORT_LOGGING_LEVEL_WARNING,
                    "OnnxTask");

                break;
        }
        case 1: {
                loader_ = std::make_shared<ONNXDynamicModelLoader>(
                    model_path_, 1, ORT_ENABLE_BASIC, ORT_LOGGING_LEVEL_WARNING,
                    "OnnxTask");

                break;
        }
        }

        if (!loader_->Load())
        {
                SPDLOG_CRITICAL("{}: model_path: {}, load failed", name_, model_path_);
                return -1;
        }

        SPDLOG_INFO("{}: model_path: {}, load success", name_, model_path_);

        return 0;
}

int OnnxTask::Process(TaskContext* ctx)
{
        // input
        Datum data_d = ctx->Inputs()[0].Value();
        SPDLOG_DEBUG("{}: Consume Datum: {}", name_, data_d);
        auto data_opt = data_d.Consume<std::vector<std::vector<float>*>>();
        if (!data_opt)
        {
                SPDLOG_CRITICAL("{}: invalid input", name_);
                return -1;
        }
        auto data = *(data_opt.value());
        // SPDLOG_DEBUG("{}: input: {}", name_, data);

        // inference
        auto inference_ret = loader_->Inference(data);
        if (!inference_ret)
        {
                SPDLOG_CRITICAL("{}: Inference failed", name_);
                return -1;
        }

        // for debug
        DumpInferenceRet(inference_ret.value());

        SPDLOG_DEBUG("{}: inference over", name_);

        ctx->Outputs()[0].AddDatum(ParseInferenceResult(inference_ret.value()));

        inference_ret.reset();

        return 0;
}

int OnnxTask::Close(TaskContext* ctx)
{
        SPDLOG_DEBUG("{}: closing ...", name_);

        return 0;
}

}   // namespace dni
