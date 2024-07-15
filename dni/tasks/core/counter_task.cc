#include "dni/framework/framework.h"
#include "dni/tasks/core/counter_task.pb.h"
#include "spdlog/spdlog.h"

namespace dni {

class CounterTask: public TaskBase {
public:
        CounterTask(): name_("CounterTask") {}
        ~CounterTask() override {}

        int Open(TaskContext* ctx) override
        {
                name_ += "(" + ctx->Name() + ")";
                SPDLOG_DEBUG("{}: open task ...", name_);

                options_ = ctx->Options<CounterTaskOptions>();
                feature_ = options_.feature();
                SPDLOG_DEBUG("{}: feature: {}", name_, feature_);

                return 0;
        }

        int Process(TaskContext* ctx) override
        {
                // input
                SPDLOG_DEBUG("{}: Consume Datum size: {}", name_, ctx->Inputs().size());

                uint64_t cnt = 0;
                auto input_size = ctx->Inputs().size();
                for (int i = 0; i < input_size; i++)
                {
                        Datum input_d = ctx->Inputs()[i].Value();

                        auto input_opt = input_d.Consume<double_t>();
                        if (!input_opt)
                        {
                                SPDLOG_CRITICAL("{}: invalid input", name_);
                                return -1;
                        }
                        auto number = *(input_opt.value());
                        SPDLOG_DEBUG("{}: number: {}", name_, number);

                        if (feature_ == number)
                        {
                                cnt++;
                        }
                }

                SPDLOG_DEBUG("{}: after calculation: {}", name_, cnt);

                ctx->Outputs()[0].AddDatum(Datum(cnt));

                return 0;
        }

        int Close(TaskContext* ctx) override
        {
                SPDLOG_DEBUG("{}: closing ...", name_);

                return 0;
        }

private:
        std::string name_;

        CounterTaskOptions options_;
        double_t feature_;
};

REGISTER(CounterTask);

}   // namespace dni
