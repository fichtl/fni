#include "dni/framework/framework.h"
#include "spdlog/spdlog.h"

namespace dni {

        class CounterTask: public TaskBase {
        public:
                CounterTask(): name_("CounterTask") {}
                ~CounterTask() override {}

                int Open(TaskContext* ctx) override
                {
                        SPDLOG_DEBUG("Task {}: open task ...", name_);

                        return 0;
                }

                int Process(TaskContext* ctx) override
                {
                        // sidedata0
                        auto spec_d = ctx->GetInputSideData()[0];
                        SPDLOG_DEBUG(
                            "Task {}: Consume side data spec: {}", name_, spec_d);

                        auto spec_opt = spec_d.Consume<double_t>();
                        if (!spec_opt)
                        {
                                SPDLOG_WARN(
                                    "Task {}: Consume() spec returns NULL, wait for "
                                    "input ...",
                                    name_);
                        }

                        auto spec_value = *(spec_opt.value());

                        SPDLOG_DEBUG("Task {}: threshold_values: {}", name_, spec_value);

                        // input
                        SPDLOG_DEBUG("Task {}: Consume Datum size: {}", name_, ctx->Inputs().size());

                        uint64_t cnt = 0;
                        for (int i = 0; i < ctx->Inputs().size(); i++)
                        {
                                Datum input_d = ctx->Inputs()[i].Value();

                                auto input_opt = input_d.Consume<double_t>();
                                if (!input_opt)
                                {
                                        SPDLOG_WARN(
                                        "Task {}: Consume() returns NULL, wait for "
                                        "input ...",
                                        name_);
                                }
                                auto number = *(input_opt.value());
                                SPDLOG_DEBUG("Task {}: number: {}", name_, number);

                                if (spec_value == number)
                                {
                                        cnt++;
                                }
                        }

                        SPDLOG_DEBUG("Task {}: after calculation: {}", name_, cnt);

                        ctx->Outputs()[0].AddDatum(Datum(cnt));

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

        REGISTER(CounterTask);

}   // namespace dni
