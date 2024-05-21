#include "onnx_model_loader.h"

namespace dni {

// TODO: err process,

ONNXModelLoader::ONNXModelLoader(
    const std::string& model, int threads, GraphOptimizationLevel optimize_level,
    OrtLoggingLevel logging_level, const std::string& logid)
{
        modelPath_ = model;

        env_ = std::make_unique<Ort::Env>(logging_level, logid.c_str());

        session_options_ = std::make_unique<Ort::SessionOptions>();
        session_options_->SetIntraOpNumThreads(threads);
        session_options_->SetGraphOptimizationLevel(optimize_level);

        session_ =
            std::make_unique<Ort::Session>(*env_, modelPath_.c_str(), *session_options_);
}

ONNXModelLoader::~ONNXModelLoader() {}

}   // namespace dni
