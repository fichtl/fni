#include <unordered_set>

#include "dni/framework/framework.h"
#include "spdlog/spdlog.h"

namespace dni {

        class NumberStatsTask: public TaskBase {
        public:
                NumberStatsTask(): name_("NumberStatsTask") {}
                ~NumberStatsTask() override {}

                int Open(TaskContext* ctx) override
                {
                        name_ += " " + ctx->Name();
                        SPDLOG_DEBUG("Task {}: open task ...", name_);
                        return 0;
                }

                int Process(TaskContext* ctx) override
                {
                        // input0, std::unordered_map<uint32_t, int>
                        Datum stats_map_d = ctx->Inputs()[0].Value();
                        SPDLOG_DEBUG("Task {}: Consume Datum: {}", name_, stats_map_d);
                        auto stats_map_opt =
                            stats_map_d.Consume<std::unordered_map<uint32_t, int>>();
                        if (!stats_map_opt)
                        {
                                SPDLOG_WARN(
                                    "Task {}: Consume() returns NULL, wait for "
                                    "input ...",
                                    name_);
                        }
                        auto stats_map = *(stats_map_opt.value());
                        SPDLOG_DEBUG("Task {}: stats_map: {}", name_, stats_map);

                        // numKeyLen
                        auto numKeyLen = stats_map.size();
                        SPDLOG_DEBUG("Task {}: numKeyLen: {}", name_, numKeyLen);

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
                                SPDLOG_DEBUG("Task {}: keyDiffs.insert: {}", name_, keys[i + 1] - keys[i]);
                        }
                        auto keySeriesDiffTypeNum = keyDiffs.size();

                        SPDLOG_DEBUG(
                            "Task {}: keySeriesDiffTypeNum: {}",
                            name_,
                            keySeriesDiffTypeNum);

                        // sidedata0
                        auto numValueSum_d = ctx->GetInputSideData()[0];
                        SPDLOG_DEBUG(
                            "Task {}: Consume side data extra_number: {}",
                            name_,
                            numValueSum_d);

                        auto numValueSum_opt = numValueSum_d.Consume<int>();
                        if (!numValueSum_opt)
                        {
                                SPDLOG_WARN(
                                    "Task {}: Consume() returns NULL, wait for "
                                    "input ...",
                                    name_);
                        }

                        int numValueSum = *(numValueSum_opt.value());
                        SPDLOG_DEBUG("Task {}: numValueSum: {}", name_, numValueSum);

                        // sidedata1
                        auto ratioMin_d = ctx->GetInputSideData()[1];
                        SPDLOG_DEBUG(
                            "Task {}: Consume side data ratioMin_d: {}",
                            name_,
                            ratioMin_d);

                        auto ratioMin_opt = ratioMin_d.Consume<float_t>();
                        if (!ratioMin_opt)
                        {
                                SPDLOG_WARN(
                                    "Task {}: Consume() returns NULL, wait for "
                                    "input ...",
                                    name_);
                        }

                        auto ratioMin = *(ratioMin_opt.value());
                        SPDLOG_DEBUG("Task {}: ratioMin: {}", name_, ratioMin);

                        // sidedata2
                        auto ratioMax_d = ctx->GetInputSideData()[2];
                        SPDLOG_DEBUG(
                            "Task {}: Consume side data ratioMax_d: {}",
                            name_,
                            ratioMax_d);

                        auto ratioMax_opt = ratioMax_d.Consume<float_t>();
                        if (!ratioMax_opt)
                        {
                                SPDLOG_WARN(
                                    "Task {}: Consume() returns NULL, wait for "
                                    "input ...",
                                    name_);
                        }

                        auto ratioMax = *(ratioMax_opt.value());
                        SPDLOG_DEBUG("Task {}: ratioMax: {}", name_, ratioMax);

                        // sidedata3
                        auto score_thresholds_d = ctx->GetInputSideData()[3];
                        SPDLOG_DEBUG(
                            "Task {}: Consume side data score_thresholds_d: {}",
                            name_,
                            score_thresholds_d);

                        auto score_thresholds_opt =
                            score_thresholds_d.Consume<std::vector<float_t>>();
                        if (!score_thresholds_opt)
                        {
                                SPDLOG_WARN(
                                    "Task {}: Consume() returns NULL, wait for "
                                    "input ...",
                                    name_);
                        }

                        auto score_thresholds = *(score_thresholds_opt.value());
                        SPDLOG_DEBUG(
                            "Task {}: score_thresholds: {}", name_, score_thresholds);

                        // score
                        float_t score = 0.0;
                        float_t min_ = (float_t) numValueSum * ratioMin;
                        float_t max_ = (float_t) numValueSum * ratioMax;
                        if (numKeyLen < min_)
                        {
                                score = score_thresholds[0];
                        }
                        else
                        {
                                if (keySeriesDiffTypeNum <= min_)
                                {
                                        score = score_thresholds[1];
                                }
                                else if (keySeriesDiffTypeNum >= max_)
                                {
                                        score = score_thresholds[2];
                                }
                                else
                                {
                                        score = score_thresholds[3];
                                }
                        }

                        SPDLOG_DEBUG("Task {}: after calculation: {}", name_, score);

                        ctx->Outputs()[0].AddDatum(Datum(score));

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

        REGISTER(NumberStatsTask);

}   // namespace dni
