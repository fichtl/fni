#include <algorithm>

#include "dni/framework/framework.h"
#include "spdlog/spdlog.h"

namespace dni {

class MaxTask: public TaskBase {
public:
        MaxTask(): name_("MaxTask") {}
        ~MaxTask() override {}

        int Open(TaskContext* ctx) override
        {
                name_ += "(" + ctx->Name() + ")";
                SPDLOG_DEBUG("{}: open task ...", name_);

                return 0;
        }

        int Process(TaskContext* ctx) override
        {
                // input
                SPDLOG_DEBUG("{}: Consume Datum: {}", name_, ctx->Inputs().size());

                std::vector<double_t> numbers;
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
                        SPDLOG_DEBUG("{}: val: {}", name_, number);

                        numbers.push_back(number);
                }

                // max
                auto maxPos = std::max_element(numbers.begin(), numbers.end());
                SPDLOG_DEBUG("{}: after calculation: {}", name_, *maxPos);

                ctx->Outputs()[0].AddDatum(Datum(*maxPos));

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

REGISTER(MaxTask);

}   // namespace dni
