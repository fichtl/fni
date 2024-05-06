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
                        auto threshold_configs_d = ctx->GetInputSideData()[0];
                        SPDLOG_DEBUG(
                            "Task {}: Consume side data threshold_configs: {}",
                            name_,
                            threshold_configs_d);

                        auto threshold_configs_opt =
                            threshold_configs_d.Consume<std::vector<double_t>>();
                        if (!threshold_configs_opt)
                        {
                                SPDLOG_WARN(
                                    "Task {}: Consume() returns NULL, wait for "
                                    "input ...",
                                    name_);
                        }

                        auto threshold_configs = *(threshold_configs_opt.value());

                        SPDLOG_DEBUG(
                            "Task {}: threshold_values: {}", name_, threshold_configs);

                        // sidedata1
                        auto score_configs_d = ctx->GetInputSideData()[1];
                        SPDLOG_DEBUG(
                            "Task {}: Consume side data score_configs: {}",
                            name_,
                            score_configs_d);

                        auto score_configs_opt =
                            score_configs_d.Consume<std::vector<double_t>>();
                        if (!score_configs_opt)
                        {
                                SPDLOG_WARN(
                                    "Task {}: Consume() returns NULL, wait for "
                                    "input ...",
                                    name_);
                        }

                        auto score_configs = *(score_configs_opt.value());

                        SPDLOG_DEBUG("Task {}: score_values: {}", name_, score_configs);

                        SPDLOG_DEBUG(
                            "Task {}: sidedata config size: {}, {}",
                            name_,
                            threshold_configs.size(),
                            score_configs.size());

                        // sidedata2, condition, support multi conditions
                        auto conditions_d = ctx->GetInputSideData()[2];
                        SPDLOG_DEBUG(
                            "Task {}: Consume side data conditions: {}",
                            name_,
                            conditions_d);

                        auto conditions_opt =
                            conditions_d.Consume<std::vector<double_t>>();
                        if (!conditions_opt)
                        {
                                SPDLOG_WARN(
                                    "Task {}: Consume() returns NULL, wait for "
                                    "input ...",
                                    name_);
                        }

                        auto conditions = *(conditions_opt.value());

                        SPDLOG_DEBUG("Task {}: score_values: {}", name_, conditions);

                        // input
                        Datum d = ctx->Inputs()[0].Value();
                        SPDLOG_DEBUG("Task {}: Consume Datum: {}", name_, d);
                        auto opt = d.Consume<double_t>();
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
                        auto cond_val_opt = cond_val_d.Consume<std::vector<double_t>>();
                        if (!cond_val_opt)
                        {
                                SPDLOG_WARN(
                                    "Task {}: Consume() returns NULL, wait for "
                                    "input ...",
                                    name_);
                        }
                        auto cond_vals = *(cond_val_opt.value());
                        SPDLOG_DEBUG("Task {}: val: {}", name_, cond_vals);

                        if (cond_vals.size() != conditions.size())
                        {
                                SPDLOG_CRITICAL(
                                    "Task {}: conditions number not equal, {} vs {}",
                                    name_, cond_vals.size(), conditions.size());

                                ctx->Outputs()[0].AddDatum(Datum(double_t(-1.00)));

                                return 0;
                        }

                        // score
                        bool cond_match = true;
                        for (size_t i = 0; i < cond_vals.size(); i++)
                        {
                                if (cond_vals[i] < conditions[i])
                                {
                                        cond_match = false;
                                        break;
                                }
                        }

                        double_t score = 0.0;
                        if (cond_match)
                        {
                                if (val <= threshold_configs[0])
                                {
                                        score = score_configs[0];
                                }
                                else if (val >= threshold_configs[1])
                                {
                                        score = score_configs[1];
                                }
                        }

                        SPDLOG_DEBUG("Task {}: after calculation: {}", name_, score);

                        ctx->Outputs()[0].AddDatum(Datum(score));

                        return 0;
                }

                int Close(TaskContext* ctx) override
                {
                        SPDLOG_DEBUG("Task {}: closing ...", name_);

                        return 0;
                }

        private:
                std::string name_;
        };

        REGISTER(ConditionThresholdTask);

}   // namespace dni
