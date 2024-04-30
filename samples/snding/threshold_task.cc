#include "dni/framework/framework.h"
#include "spdlog/spdlog.h"

namespace dni {

        class ThresholdTask: public TaskBase {
        public:
                ThresholdTask(): name_("ThresholdTask") {}
                ~ThresholdTask() override {}

                int Open(TaskContext* ctx) override
                {
                        SPDLOG_DEBUG("Task {}: open task ...", name_);

                        return 0;
                }

                int Process(TaskContext* ctx) override
                {
                        SPDLOG_DEBUG(
                            "Task {}: ctx->GetInputSideData().size(): {}",
                            name_,
                            ctx->GetInputSideData().size());

                        // sidedata0
                        auto threshold_configs = ctx->GetInputSideData()[0];
                        SPDLOG_DEBUG(
                            "Task {}: Consume side data extra_number: {}",
                            name_,
                            threshold_configs);

                        auto th_opt = threshold_configs.Consume<std::vector<uint64_t>>();
                        if (!th_opt)
                        {
                                SPDLOG_WARN(
                                    "Task {}: Consume() returns NULL, wait for "
                                    "input ...",
                                    name_);
                        }

                        threshold_values_ = *(th_opt.value());

                        // TODO: sort

                        SPDLOG_DEBUG(
                            "Task {}: threshold_values: {}", name_, threshold_values_);

                        // sidedata1
                        auto score_configs = ctx->GetInputSideData()[1];
                        SPDLOG_DEBUG(
                            "Task {}: Consume side data environment: {}",
                            name_,
                            score_configs);

                        auto score_opt = score_configs.Consume<std::vector<float_t>>();
                        if (!score_opt)
                        {
                                SPDLOG_WARN(
                                    "Task {}: Consume() returns NULL, wait for "
                                    "input ...",
                                    name_);
                        }

                        score_values_ = *(score_opt.value());

                        // TODO: sort

                        SPDLOG_DEBUG("Task {}: score_values: {}", name_, score_values_);

                        SPDLOG_DEBUG(
                            "Task {}: sidedata config size: {}, {}",
                            name_,
                            threshold_values_.size(),
                            score_values_.size());

                        // input
                        Datum d = ctx->Inputs()[0].Value();
                        SPDLOG_DEBUG("Task {}: Consume Datum: {}", name_, d);
                        auto opt = d.Consume<uint64_t>();
                        if (!opt)
                        {
                                SPDLOG_WARN(
                                    "Task {}: Consume() returns NULL, wait for "
                                    "input ...",
                                    name_);
                        }
                        auto val = *(opt.value());
                        SPDLOG_DEBUG("Task {}: val: {}", name_, val);

                        // score
                        size_t i;
                        for (i = 0; i < threshold_values_.size(); i++)
                        {
                                if (val < threshold_values_[i])
                                {
                                        break;
                                }
                        }

                        auto score = score_values_[i];

                        SPDLOG_DEBUG("Task {}: after calculation: {}", name_, score);

                        ctx->Outputs()[0].AddDatum(Datum(score));

                        return 0;
                }

                int Close(TaskContext* ctx) override
                {
                        SPDLOG_DEBUG("Task {}: closing ...", name_);
                        threshold_values_.clear();
                        score_values_.clear();

                        return 0;
                }

        private:
                std::string name_;
                std::vector<uint64_t> threshold_values_;
                std::vector<float_t> score_values_;
        };

        REGISTER(ThresholdTask);

}   // namespace dni
