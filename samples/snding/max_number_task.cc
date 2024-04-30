#include <algorithm>

#include "dni/framework/framework.h"
#include "spdlog/spdlog.h"

namespace dni {

        class MaxNumberTask: public TaskBase {
        public:
                MaxNumberTask(): name_("MaxNumberTask") {}
                ~MaxNumberTask() override {}

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

                        // max
                        auto maxPos = max_element(numbers.begin(), numbers.end());
                        SPDLOG_DEBUG("Task {}: after calculation: {}", name_, *maxPos);

                        ctx->Outputs()[0].AddDatum(Datum(*maxPos));

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

        REGISTER(MaxNumberTask);

}   // namespace dni
