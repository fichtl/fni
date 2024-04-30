#include "dni/framework/framework.h"
#include "spdlog/spdlog.h"

namespace dni {

        class SumTask: public TaskBase {
        public:
                SumTask(): name_("SumTask") {}
                ~SumTask() override {}

                int Open(TaskContext* ctx) override
                {
                        SPDLOG_DEBUG("Task {}: open task ...", name_);

                        return 0;
                }

                int Process(TaskContext* ctx) override
                {
                        // input
                        Datum d = ctx->Inputs()[0].Value();
                        SPDLOG_DEBUG("Task {}: Consume Datum: {}", name_, d);
                        auto opt = d.Consume<std::vector<float_t>>();
                        if (!opt)
                        {
                                SPDLOG_WARN(
                                    "Task {}: Consume() returns NULL, wait for "
                                    "input ...",
                                    name_);
                        }
                        auto numbers = *(opt.value());
                        SPDLOG_DEBUG("Task {}: val: {}", name_, numbers);

                        // sum
                        float_t sum = 0.0;
                        for (size_t i = 0; i < numbers.size(); i++)
                        {
                                sum += numbers[i];
                        }

                        SPDLOG_DEBUG("Task {}: after calculation: {}", name_, sum);

                        ctx->Outputs()[0].AddDatum(Datum(sum));

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

        REGISTER(SumTask);

}   // namespace dni
