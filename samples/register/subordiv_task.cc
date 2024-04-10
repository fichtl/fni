#include "dni/framework/framework.h"
#include "spdlog/spdlog.h"

namespace dni {

        class SubOrDivTask: public TaskBase {
        public:
                SubOrDivTask(): name_("SubOrDivTask") {}
                ~SubOrDivTask() override {}

                int Open(TaskContext* ctx) override
                {
                        name_ += " " + ctx->Name();
                        SPDLOG_DEBUG("Task {}: open task ...", name_);
                        return 0;
                }

                int Process(TaskContext* ctx) override
                {
__WAIT:

                        for (size_t i = 0; i < ctx->Inputs().size(); i++)
                        {
                                Datum d = ctx->Inputs()[i].Value();
                                auto opt = d.Consume<int>();
                                if (opt == std::nullopt)
                                {
                                        SPDLOG_WARN(
                                            "Task {}: Consume() returns NULL, wait for "
                                            "input ...",
                                            name_);
                                        std::this_thread::sleep_for(
                                            std::chrono::milliseconds(300));
                                        goto __WAIT;
                                }

                                ctx->Inputs()[i].Pop();

                                auto val = *(opt.value());
                                if (i == 0)
                                {
                                        val--;
                                }
                                else
                                {
                                        val /= 10;
                                }
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

        REGISTER(SubOrDivTask);

}   // namespace dni
