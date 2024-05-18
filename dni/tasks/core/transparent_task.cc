#include "dni/framework/framework.h"
#include "dni/tasks/core/transparent_task.pb.h"
#include "fmt/color.h"
#include "fmt/ranges.h"
#include "spdlog/spdlog.h"

namespace dni {

class TransparentTask: public TaskBase {
public:
        TransparentTask(): name_("TransparentTask") {}
        ~TransparentTask() override {}

        int Open(TaskContext* ctx) override
        {
                name_ += "(" + ctx->Name() + ")";
                SPDLOG_DEBUG("{}: open task ...", name_);

                options_ = ctx->Options<TransparentTaskOptions>();

                return 0;
        }

        int Process(TaskContext* ctx) override
        {
                for (auto observe : options_.observe())
                {
                        int id = ctx->Inputs().Slot(observe.tag(), observe.index());
                        if (id < 0)
                        {
                                SPDLOG_INFO(
                                    "{}: {} \"{}:{}\"", name_,
                                    fmt::styled("✘", fmt::fg(fmt::color::red)),
                                    observe.tag(), observe.index());
                                continue;
                        }
                        SPDLOG_INFO(
                            "{}: {} \"{}:{}\" {}", name_,
                            fmt::styled("✔︎", fmt::fg(fmt::color::green)), observe.tag(),
                            observe.index(), ctx->Inputs()[id].Name());
                }

                for (size_t i = 0; i < ctx->Inputs().size(); i++)
                {
                        if (i >= ctx->Outputs().size())
                        {
                                SPDLOG_DEBUG("input: {}", ctx->Inputs()[i].Name());
                                continue;
                        }
                        SPDLOG_DEBUG(
                            "input:{} -> output:{}", ctx->Inputs()[i].Name(),
                            ctx->Outputs()[i].Name());
                        ctx->Outputs()[i].AddDatum(ctx->Inputs()[i].Value());
                }
        }

        int Close(TaskContext* ctx) override
        {
                SPDLOG_DEBUG("{}: closing ...", name_);
                return 0;
        }

private:
        std::string name_;

        TransparentTaskOptions options_;
};

REGISTER(TransparentTask);

}   // namespace dni
