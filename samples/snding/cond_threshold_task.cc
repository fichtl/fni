#include "dni/framework/framework.h"
#include "spdlog/spdlog.h"

namespace dni {

        class ConditionThresholdTask: public TaskBase {
        public:
                ConditionThresholdTask(): name_("ConditionThresholdTask") {}
                ~ConditionThresholdTask() override {}

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

                        SPDLOG_DEBUG("Task {}: score_values: {}", name_, score_values_);

                        SPDLOG_DEBUG(
                            "Task {}: sidedata config size: {}, {}",
                            name_,
                            threshold_values_.size(),
                            score_values_.size());

                        // sidedata2, condition, support multi conditions
                        auto cond = ctx->GetInputSideData()[2];
                        SPDLOG_DEBUG(
                            "Task {}: Consume side data environment: {}",
                            name_,
                            cond);

                        auto cond_opt = cond.Consume<std::vector<uint64_t>>();
                        if (!cond_opt)
                        {
                                SPDLOG_WARN(
                                    "Task {}: Consume() returns NULL, wait for "
                                    "input ...",
                                    name_);
                        }

                        cond_threshold_values_ = *(cond_opt.value());

                        SPDLOG_DEBUG("Task {}: score_values: {}", name_, cond_threshold_values_);

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

                        // input condition
                        Datum cond_val_d = ctx->Inputs()[1].Value();
                        SPDLOG_DEBUG("Task {}: Consume Datum: {}", name_, cond_val_d);
                        auto cond_val_opt = cond_val_d.Consume<std::vector<uint64_t>>();
                        if (!cond_val_opt)
                        {
                                SPDLOG_WARN(
                                    "Task {}: Consume() returns NULL, wait for "
                                    "input ...",
                                    name_);
                        }
                        auto cond_vals = *(cond_val_opt.value());
                        SPDLOG_DEBUG("Task {}: val: {}", name_, cond_vals);

                        // score
                        bool cond_match = true;
                        for (size_t i = 0; i < cond_vals.size(); i++)
                        {
                                if (cond_vals[i] < cond_threshold_values_[i])
                                {
                                        cond_match = false;
                                        break;
                                }
                                
                        }

                        float_t score = 0.0;
                        if (cond_match)
                        {
                                if (val <= threshold_values_[0])
                                {
                                        score = score_values_[0];
                                }
                                else if (val >= threshold_values_[1])
                                {
                                        score = score_values_[1];
                                }               
                        }

                        SPDLOG_DEBUG("Task {}: after calculation: {}", name_, score);

                        ctx->Outputs()[0].AddDatum(Datum(score));

                        return 0;
                }

                int Close(TaskContext* ctx) override
                {
                        SPDLOG_DEBUG("Task {}: closing ...", name_);
                        threshold_values_.clear();

                        return 0;
                }

        private:
                std::string name_;
                std::vector<uint64_t> threshold_values_;
                std::vector<float_t> score_values_;
                std::vector<uint64_t> cond_threshold_values_;
        };

        REGISTER(ConditionThresholdTask);

}   // namespace dni
