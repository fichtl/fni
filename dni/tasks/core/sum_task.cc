#include "dni/framework/framework.h"
#include "spdlog/spdlog.h"

namespace dni {

class SumTask: public TaskBase {
public:
        SumTask(): name_("SumTask") {}
        ~SumTask() override {}

        int Open(TaskContext* ctx) override
        {
                name_ += "(" + ctx->Name() + ")";
                SPDLOG_DEBUG("{}: open task ...", name_);

                return 0;
        }

        int Process(TaskContext* ctx) override
        {
                SPDLOG_DEBUG("{}: Datum size: {}", name_, ctx->Inputs().size());

                double_t sum = 0.0;
                auto input_size = ctx->Inputs().size();
                for (int i = 0; i < input_size; i++)
                {
                        Datum d = ctx->Inputs()[i].Value();

                        auto opt = d.Consume<double_t>();
                        if (!opt)
                        {
                                SPDLOG_CRITICAL("{}: invalid input", name_);
                                return -1;
                        }
                        auto number = *(opt.value());
                        SPDLOG_DEBUG("{}: number: {}", name_, number);

                        sum += number;
                }

                SPDLOG_DEBUG("{}: after calculation: {}", name_, sum);

                ctx->Outputs()[0].AddDatum(Datum(sum));

                return 0;
        }

        int Close(TaskContext* ctx) override
        {
                SPDLOG_DEBUG("{}: closing ...", name_);

                return 0;
        }

private:
        std::string name_;
};

REGISTER(SumTask);

}   // namespace dni
