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
                                    "Task {}: Consume() threshold_configs returns NULL, "
                                    "wait for input ...",
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
                                    "Task {}: Consume() score_configs returns NULL, wait "
                                    "for input ...",
                                    name_);
                        }

                        std::vector<double_t> score_configs =
                            *(score_configs_opt.value());

                        SPDLOG_DEBUG("Task {}: score_values: {}", name_, score_configs);

                        SPDLOG_DEBUG(
                            "Task {}: sidedata config size: {}, {}",
                            name_,
                            threshold_configs.size(),
                            score_configs.size());

                        // input
                        Datum statistic_d = ctx->Inputs()[0].Value();
                        SPDLOG_DEBUG("Task {}: Consume Datum: {}", name_, statistic_d);
                        auto statistic_opt = statistic_d.Consume<double_t>();
                        if (!statistic_opt)
                        {
                                SPDLOG_WARN(
                                    "Task {}: Consume() statistic returns NULL, wait for "
                                    "input ...",
                                    name_);
                        }
                        auto statistic = *(statistic_opt.value());
                        SPDLOG_DEBUG("Task {}: statistic: {}", name_, statistic);

                        // score
                        size_t i;
                        for (i = 0; i < threshold_configs.size(); i++)
                        {
                                if (statistic < threshold_configs[i])
                                {
                                        break;
                                }
                        }

                        auto score = score_configs[i];

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

        REGISTER(ThresholdTask);

}   // namespace dni
