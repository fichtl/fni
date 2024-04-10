#include "dni/framework/framework.h"
#include "spdlog/spdlog.h"

namespace dni {

        class AddInputSideDataTask: public TaskBase {
        public:
                AddInputSideDataTask(): name_("AddInputSideDataTask") {}
                ~AddInputSideDataTask() override {}

                int Open(TaskContext* ctx) override
                {
                        name_ += " " + ctx->Name();
                        SPDLOG_DEBUG("Task {}: open task ...", name_);
                        return 0;
                }

                int Process(TaskContext* ctx) override
                {
                        Datum d = ctx->Inputs()[0].Value();
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

                        auto sidedata = ctx->GetInputSideData()[0];
                        SPDLOG_DEBUG("Task {}: Consume side data: {}", name_, sidedata);
                        auto sidedata_opt = sidedata.Consume<int>();
                        if (!sidedata_opt)
                        {
                                SPDLOG_WARN(
                                    "Task {}: Consume() returns NULL, wait for "
                                    "input ...",
                                    name_);
                        }
                        int sidedata_val = *(sidedata_opt.value());

                        val++;
                        val += sidedata_val;
                        SPDLOG_DEBUG("Task {}: after calculation: {}", name_, val);

                        ctx->Outputs()[0].AddDatum(Datum(val));

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

        REGISTER(AddInputSideDataTask);

}   // namespace dni