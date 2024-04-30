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
                        SPDLOG_DEBUG(
                            "Task {}: ctx->GetInputSideData().size(): {}",
                            name_,
                            ctx->GetInputSideData().size());

                        // sidedata0
                        auto spec = ctx->GetInputSideData()[0];
                        SPDLOG_DEBUG(
                            "Task {}: Consume side data extra_number: {}",
                            name_,
                            spec);

                        auto spec_opt = spec.Consume<uint64_t>();
                        if (!spec_opt)
                        {
                                SPDLOG_WARN(
                                    "Task {}: Consume() returns NULL, wait for "
                                    "input ...",
                                    name_);
                        }

                        auto spec_value_ = *(spec_opt.value());

                        SPDLOG_DEBUG(
                            "Task {}: threshold_values: {}", name_, spec_value_);

                        // input
                        Datum d = ctx->Inputs()[0].Value();
                        SPDLOG_DEBUG("Task {}: Consume Datum: {}", name_, d);
                        auto opt = d.Consume<std::vector<uint64_t>>();
                        if (!opt)
                        {
                                SPDLOG_WARN(
                                    "Task {}: Consume() returns NULL, wait for "
                                    "input ...",
                                    name_);
                        }
                        auto numbers = *(opt.value());
                        SPDLOG_DEBUG("Task {}: numbers: {}", name_, numbers);

                        // count
                        uint64_t cnt = 0;
                        for (size_t i = 0; i < numbers.size(); i++)
                        {
                                if (spec_value_ == numbers[i])
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

                uint64_t spec_value_;
        };

        REGISTER(CounterTask);

}   // namespace dni
