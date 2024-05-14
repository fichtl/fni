#include "dni/framework/framework.h"
#include "spdlog/spdlog.h"

namespace dni {

// Inputs: packet/netdev/resource result
//  vector of scores (double)
// Outputs:
//  -1: input wrong;
//  0: normal
//  1~4: abnormal
class SndAdTask: public TaskBase {
public:
        SndAdTask(): name_("SndAdTask") {}
        ~SndAdTask() override {}

        int Open(TaskContext* ctx) override
        {
                name_ += "(" + ctx->Name() + ")";
                SPDLOG_DEBUG("{}: open task ...", name_);

                return 0;
        }

        int Process(TaskContext* ctx) override
        {
                if (ctx->Inputs().size() != 3)
                {
                        SPDLOG_CRITICAL(
                            "{}: input size is not 3, is: {}",
                            name_,
                            ctx->Inputs().size());
                        return -1;
                }

                std::vector<int> judges;
                int sum = 0;
                for (int i = 0; i < ctx->Inputs().size(); i++)
                {
                        Datum judge_d = ctx->Inputs()[i].Value();
                        SPDLOG_DEBUG("{}: Consume Datum: {}", name_, judge_d);
                        auto judge_opt = judge_d.Consume<double_t>();
                        if (!judge_opt)
                        {
                                SPDLOG_CRITICAL("{}: invalid input", name_);
                                return -1;
                        }
                        auto judge = *(judge_opt.value());
                        SPDLOG_DEBUG("{}: judge: {}", name_, judge);

                        sum += (int(judge + 0.5));
                        judges.push_back(int(judge + 0.5));
                }

                int abnormal_type = 0;   // -1, normal
                if (sum == 3)
                {
                        abnormal_type = 1;
                }
                else if (sum == 2)
                {
                        if (judges[0] + judges[1] == 2)
                        {
                                abnormal_type = 2;
                        }
                        else if (judges[0] + judges[2] == 2)
                        {
                                abnormal_type = 3;
                        }
                        else if (judges[1] + judges[2] == 2)
                        {
                                abnormal_type = 4;
                        }
                }

                SPDLOG_DEBUG("{}: after calculation: {}", name_, abnormal_type);

                ctx->Outputs()[0].AddDatum(Datum(abnormal_type));

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

REGISTER(SndAdTask);

}   // namespace dni
