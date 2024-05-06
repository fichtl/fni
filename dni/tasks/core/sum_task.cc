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
                        SPDLOG_DEBUG(
                            "Task {}: Datum size: {}", name_, ctx->Inputs().size());

                        double_t sum = 0.0;
                        for (int i = 0; i < ctx->Inputs().size(); i++)
                        {
                                Datum score_d = ctx->Inputs()[i].Value();

                                auto score_opt = score_d.Consume<double_t>();
                                if (!score_opt)
                                {
                                        SPDLOG_WARN(
                                            "Task {}: Consume() Datum returns NULL, wait "
                                            "for input ...",
                                            name_);
                                }
                                auto score = *(score_opt.value());
                                SPDLOG_DEBUG("Task {}: scores: {}", name_, score);

                                sum += score;
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
