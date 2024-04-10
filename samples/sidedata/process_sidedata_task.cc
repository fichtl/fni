#include "dni/framework/framework.h"
#include "spdlog/spdlog.h"

namespace dni {

        class ProcessSideDataTask: public TaskBase {
        public:
                ProcessSideDataTask(): name_("ProcessSideDataTask") {}
                ~ProcessSideDataTask() override {}

                int Open(TaskContext* ctx) override
                {
                        name_ += " " + ctx->Name();
                        SPDLOG_DEBUG("Task {}: open task ...", name_);
                        return 0;
                }

                int Process(TaskContext* ctx) override
                {
                        //
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
                        SPDLOG_DEBUG("Task {}: val: {}", name_, val);

                        //
                        SPDLOG_DEBUG(
                            "Task {}: ctx->GetInputSideData().size(): {}",
                            name_,
                            ctx->GetInputSideData().size());

                        auto sidedata_extra_number = ctx->GetInputSideData()[0];
                        SPDLOG_DEBUG(
                            "Task {}: Consume side data extra_number: {}",
                            name_,
                            sidedata_extra_number);

                        auto sidedata_extra_number_opt =
                            sidedata_extra_number.Consume<int>();
                        if (!sidedata_extra_number_opt)
                        {
                                SPDLOG_WARN(
                                    "Task {}: Consume() returns NULL, wait for "
                                    "input ...",
                                    name_);
                        }

                        int sidedata_extra_number_val =
                            *(sidedata_extra_number_opt.value());
                        SPDLOG_DEBUG(
                            "Task {}: sidedata_extra_number_val: {}",
                            name_,
                            sidedata_extra_number_val);

                        //
                        auto sidedata_environment = ctx->GetInputSideData()[1];
                        SPDLOG_DEBUG(
                            "Task {}: Consume side data environment: {}",
                            name_,
                            sidedata_environment);

                        auto sidedata_environment_opt =
                            sidedata_environment.Consume<int>();
                        if (!sidedata_environment_opt)
                        {
                                SPDLOG_WARN(
                                    "Task {}: Consume() returns NULL, wait for "
                                    "input ...",
                                    name_);
                        }

                        int sidedata_environment_val =
                            *(sidedata_environment_opt.value());
                        SPDLOG_DEBUG(
                            "Task {}: sidedata_environment_val: {}",
                            name_,
                            sidedata_environment_val);

                        //
                        val *= sidedata_extra_number_val;
                        val += sidedata_environment_val;
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

        REGISTER(ProcessSideDataTask);

}   // namespace dni
