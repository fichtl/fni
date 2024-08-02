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
        /* 类型：std::vector<std::vector<float>*>，表示模型推理时的数据输入，
        这里设计的是两层vector，外层的vector表示的是模型的每个输入节点，
        模型有几个输入节点，那么外层的vector的size就是几。

        内层是每个输入节点的数据在进行一维化后的数据，例如某个输入节点原始维度是[10][2][7]，
        那么该节点在内层的vector的size就是 10 * 2 * 7 = 140。

        因此模型推理时，使用的输入数据，就是将每个输入节点的数据先一维化，存入内层vector，
        然后内层vector依次加入外层vector，最后将外层vector加入onnx算子的输入即可。
        */
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
