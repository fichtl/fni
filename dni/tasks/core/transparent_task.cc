#include "dni/framework/task.h"
#include "dni/framework/task_register.h"
#include "spdlog/spdlog.h"

namespace dni {

        class TransparentTask: public TaskBase {
        public:
                TransparentTask(): name_("TransparentTask") {}
                ~TransparentTask() override {}

                int Open(TaskContext* ctx) override
                {
                        name_ += " " + ctx->Name();
                        SPDLOG_DEBUG("Task {}: open task ...", name_);
                        return 0;
                }

                int Process(TaskContext* ctx) override
                {
                        for (size_t i = 0; i < ctx->Inputs().size(); i++)
                        {
                                SPDLOG_DEBUG("Task {}: pass through datum", name_);
                                ctx->Outputs()[i].AddDatum(ctx->Inputs()[i].Value());
                        }
                }

                int Close(TaskContext* ctx) override
                {
                        SPDLOG_DEBUG("Task {}: closing ...", name_);
                        return 0;
                }

        private:
                std::string name_;
        };

        REGISTER(TransparentTask);

}   // namespace dni
