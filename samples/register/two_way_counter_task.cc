#include "dni/framework/task.h"
#include "dni/framework/register.h"

class TwoWayCountTask: public dni::TaskBase {
public:
        TwoWayCountTask(): name_("TwoWayCountTask") {}
        ~TwoWayCountTask() override {}

        int Open(dni::Context* ctx) override
        {
                std::cout << "[" << name_ << "]"
                          << "Open two way count task ..." << std::endl;
                return 0;
        }

        int Process(dni::Context* ctx) override
        {
                std::cout << "[" << name_ << "]"
                          << "Process two way count task, wait for input ..."
                          << std::endl;

__WAIT:

                for (size_t i = 0; i < ctx->Inputs().size(); i++)
                {
                        dni::Datum d = ctx->Inputs()[i].Value();
                        auto opt = d.Consume<int>();
                        if (opt == std::nullopt)
                        {
                                std::cout << "[" << name_ << "]"
                                          << "Process task: Consume() return std::nullopt"
                                          << std::endl;

                                std::this_thread::sleep_for(
                                    std::chrono::milliseconds(300));
                                goto __WAIT;
                        }

                        ctx->Inputs()[i].Pop();
                        auto val = *(opt.value());

                        if (i == 0)
                        {
                                val++;
                        }
                        else
                        {
                                val *= 10;
                        }

                        std::cout << "[" << name_ << "]"
                                  << "Process task: Consume and Calc, val: " << val
                                  << std::endl;
                        ctx->Outputs()[i].AddDatum(dni::Datum(val));
                }
        }

        int Close(dni::Context* ctx) override
        {
                std::cout << "[" << name_ << "]"
                          << "Close task: " << std::endl;
                return 0;
        }

private:
        std::string name_;   // one node, one task, so it is node name
};


REGISTER(TwoWayCountTask)
