#include "dni/framework/framework.h"
#include "dni/tasks/core/threshold_task.pb.h"
#include "spdlog/spdlog.h"

namespace dni {

class ThresholdTask: public TaskBase {
public:
        ThresholdTask(): name_("ThresholdTask") {}
        ~ThresholdTask() override {}

        int Open(TaskContext* ctx) override
        {
                name_ += "(" + ctx->Name() + ")";
                SPDLOG_DEBUG("{}: open task ...", name_);

                options_ = ctx->Options<ThresholdTaskOptions>();
                SPDLOG_DEBUG(
                    "{}: thresh_score size: {}", name_, options_.thresh_scores_size());

                if (options_.thresh_scores_size() <= 0)
                {
                        SPDLOG_ERROR("{}: invalid thresholds", name_);
                        return -1;
                }
                for (auto t_s : options_.thresh_scores())
                {
                        tsmap_[t_s.threshold()] = t_s.score();
                }
                if (options_.has_default_())
                {
                        default_ = options_.default_();
                }

                return 0;
        }

        int Process(TaskContext* ctx) override
        {
                // input
                Datum statistic_d = ctx->Inputs()[0].Value();
                SPDLOG_DEBUG("{}: Consume Datum: {}", name_, statistic_d);
                auto statistic_opt = statistic_d.Consume<double_t>();
                if (!statistic_opt)
                {
                        SPDLOG_CRITICAL("{}: invalid input", name_);
                        return -1;
                }
                double_t statistic = *(statistic_opt.value());
                SPDLOG_DEBUG("{}: input: {}", name_, statistic);

                double_t score = default_;

                // std::map is always sorted by key
                for (auto th_score : tsmap_)
                {
                        if (statistic >= th_score.first)
                        {
                                score = th_score.second;
                        }
                        else
                        {
                                break;
                        }
                }

                SPDLOG_DEBUG("{}: after calculation: {}", name_, score);

                ctx->Outputs()[0].AddDatum(Datum(score));

                return 0;
        }

        int Close(TaskContext* ctx) override
        {
                SPDLOG_DEBUG("{}: closing ...", name_);

                return 0;
        }

private:
        std::string name_;

        ThresholdTaskOptions options_;
        std::map<double_t, double_t> tsmap_;
        double_t default_ = 0.;
};

REGISTER(ThresholdTask);

}   // namespace dni
