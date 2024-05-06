#include "dni/framework/framework.h"
#include "spdlog/spdlog.h"

namespace dni {

        class AddOneTask: public TaskBase {
        public:
                AddOneTask(): name_("AddOneTask") {}
                ~AddOneTask() override {}

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
                                Datum d = ctx->Inputs()[i].Value();
                                SPDLOG_DEBUG("Task {}: Consume Datum: {}", name_, d);
                                auto opt = d.Consume<int>();
                                if (!opt)
                                {
                                        SPDLOG_WARN(
                                            "Task {}: Consume() returns NULL, wait for "
                                            "input ...",
                                            name_);
                                }

                                int val = *(opt.value());
                                val++;
                                SPDLOG_DEBUG(
                                    "Task {}: after calculation: {}", name_, val);

                                ctx->Outputs()[i].AddDatum(Datum(val));
                        }

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

        REGISTER(AddOneTask);

}   // namespace dni
