#include <unordered_map>
#include <unordered_set>

#include "dni/framework/framework.h"
#include "dni/tasks/snding/number_stats_task.pb.h"
#include "spdlog/spdlog.h"

namespace dni {

class SndNumberStatsTask: public TaskBase {
public:
        SndNumberStatsTask(): name_("SndNumberStatsTask") {}
        ~SndNumberStatsTask() override {}

        int Open(TaskContext* ctx) override
        {
                name_ += "(" + ctx->Name() + ")";
                SPDLOG_DEBUG("{}: open task ...", name_);

                options_ = ctx->Options<SndNumberStatsTaskOptions>();
                numValueSum_ = options_.numvaluesum();
                ratioMin_ = options_.ratiomin();
                ratioMax_ = options_.ratiomax();
                if (options_.score_thresholds_size() != 4)
                {
                        SPDLOG_ERROR(
                            "{}: invalid score thresholds size, expect 4, get {}",
                            name_,
                            options_.score_thresholds_size());
                        return -1;
                }
                SPDLOG_DEBUG(
                    "{}: numValueSum: {}, ratioMin: {}, ratioMax: {}, "
                    "score_thresholds: {}",
                    name_, numValueSum_, ratioMin_, ratioMax_,
                    options_.score_thresholds_size());

                return 0;
        }

        int Process(TaskContext* ctx) override
        {
                // input0, std::unordered_map<uint32_t, int>
                Datum stats_map_d = ctx->Inputs()[0].Value();
                SPDLOG_DEBUG("{}: Consume stats_map: {}", name_, stats_map_d);
                auto stats_map_opt =
                    stats_map_d.Consume<std::unordered_map<uint32_t, int>>();
                if (!stats_map_opt)
                {
                        SPDLOG_CRITICAL("{}: invalid stats_map", name_);
                        return -1;
                }
                auto stats_map = *(stats_map_opt.value());
                SPDLOG_DEBUG("{}: stats_map: {}", name_, stats_map);

                // numKeyLen
                auto numKeyLen = stats_map.size();
                SPDLOG_DEBUG("{}: numKeyLen: {}", name_, numKeyLen);

                // keySeriesDiffTypeNum
                std::vector<uint32_t> keys;
                for (const auto& pair : stats_map)
                {
                        keys.push_back(pair.first);
                }
                std::sort(keys.begin(), keys.end());
                std::unordered_set<uint32_t> keyDiffs;
                for (size_t i = 0; i < keys.size() - 1; i++)
                {
                        keyDiffs.insert(keys[i + 1] - keys[i]);
                        SPDLOG_DEBUG(
                            "{}: keyDiffs.insert: {}", name_, keys[i + 1] - keys[i]);
                }
                auto keySeriesDiffTypeNum = keyDiffs.size();

                SPDLOG_DEBUG("{}: keySeriesDiffTypeNum: {}", name_, keySeriesDiffTypeNum);

                // score
                double_t score = 0.0;
                double_t min_ = (double_t) numValueSum_ * ratioMin_;
                double_t max_ = (double_t) numValueSum_ * ratioMax_;
                if (numKeyLen < min_)
                {
                        score = options_.score_thresholds()[0];
                }
                else
                {
                        if (keySeriesDiffTypeNum <= min_)
                        {
                                score = options_.score_thresholds()[1];
                        }
                        else if (keySeriesDiffTypeNum >= max_)
                        {
                                score = options_.score_thresholds()[2];
                        }
                        else
                        {
                                score = options_.score_thresholds()[3];
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

        SndNumberStatsTaskOptions options_;
        int numValueSum_;
        double ratioMin_;
        double ratioMax_;
};

REGISTER(SndNumberStatsTask);

}   // namespace dni
