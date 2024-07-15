#include "dni/framework/framework.h"
#include "dni/tasks/core/cond_threshold_task.pb.h"
#include "spdlog/spdlog.h"

namespace dni {

class ConditionThresholdTask: public TaskBase {
public:
        ConditionThresholdTask(): name_("ConditionThresholdTask") {}
        ~ConditionThresholdTask() override {}

        int Open(TaskContext* ctx) override
        {
                name_ += "(" + ctx->Name() + ")";
                SPDLOG_DEBUG("{}: open task ...", name_);

                options_ = ctx->Options<CondThresholdTaskOptions>();
                SPDLOG_DEBUG(
                    "{}: conditions size: {}, thresh_score size: {}", name_,
                    options_.conditions_size(), options_.thresh_scores_size());

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
                // input: statistics
                Datum stat_d = ctx->Inputs().Tag("STAT").Value();
                SPDLOG_DEBUG("{}: Consume Datum: {}", name_, stat_d);
                auto stat_opt = stat_d.Consume<double_t>();
                if (!stat_opt)
                {
                        SPDLOG_CRITICAL("{}: invalid input", name_);
                        return -1;
                }
                auto stat = *(stat_opt.value());
                SPDLOG_DEBUG("{}: input: {}", name_, stat);

                // input: conditions
                Datum cond_val_d = ctx->Inputs().Tag("COND").Value();
                SPDLOG_DEBUG("{}: Consume Datum: {}", name_, cond_val_d);
                auto cond_val_opt = cond_val_d.Consume<std::vector<double_t>>();
                if (!cond_val_opt)
                {
                        SPDLOG_CRITICAL("{}: invalid cond_vals", name_);
                        return -1;
                }
                auto cond_vals = *(cond_val_opt.value());
                SPDLOG_DEBUG("{}: conditions: {}", name_, cond_vals);

                if (cond_vals.size() != options_.conditions_size())
                {
                        SPDLOG_CRITICAL(
                            "{}: conditions number not equal, {} vs {}", name_,
                            cond_vals.size(), options_.conditions_size());

                        ctx->Outputs()[0].AddDatum(Datum(double_t(-1.00)));

                        return 0;
                }

                // scoring
                double_t score = default_;
                auto cond_size = cond_vals.size();
                for (size_t i = 0; i < cond_size; i++)
                {
                        if (cond_vals[i] < options_.conditions(i))
                        {
                                goto emit;
                        }
                }

                // std::map is always sorted by key
                for (auto th_score : tsmap_)
                {
                        if (stat >= th_score.first)
                        {
                                score = th_score.second;
                        }
                        else
                        {
                                break;
                        }
                }

emit:
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

        CondThresholdTaskOptions options_;
        std::map<double_t, double_t> tsmap_;
        double_t default_ = 0.;
};

REGISTER(ConditionThresholdTask);

}   // namespace dni
