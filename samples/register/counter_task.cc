#include "dni/framework/register.h"
#include "dni/framework/task.h"
#include "spdlog/spdlog.h"

class CounterTask: public dni::TaskBase {
public:
        CounterTask(): name_("CounterTask") {}
        ~CounterTask() override {}

        int Open(dni::Context* ctx) override
        {
                name_ += " " + ctx->Name();
                SPDLOG_DEBUG("Task {}: open task ...", name_);
                return 0;
        }

        int Process(dni::Context* ctx) override
        {
__WAIT:

                for (size_t i = 0; i < ctx->Inputs().size(); i++)
                {
                        dni::Datum d = ctx->Inputs()[i].Value();
                        auto opt = d.Consume<int>();
                        if (!opt)
                        {
                                spdlog::warn(
                                    "Task {}: Consume() returns NULL, wait for input ...",
                                    name_);
                                std::this_thread::sleep_for(
                                    std::chrono::milliseconds(300));
                                goto __WAIT;
                        }
                        ctx->Inputs()[i].Pop();
                        int val = *(opt.value());
                        val++;
                        SPDLOG_DEBUG("Task {}: Consume and Calc, Val: {}", name_, val);
                        ctx->Outputs()[i].AddDatum(dni::Datum(val));
                }
        }

        int Close(dni::Context* ctx) override
        {
                std::cout << "[" << name_ << "]" << "Close task: " << std::endl;
                return 0;
        }

private:
        std::string name_;   // one node, one task, so it is node name
};

REGISTER(CounterTask)
