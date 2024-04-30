#include "dni/framework/framework.h"
#include "spdlog/spdlog.h"

namespace dni {

        class ProtocolStatsTask: public TaskBase {
        public:
                ProtocolStatsTask(): name_("ProtocolStatsTask") {}
                ~ProtocolStatsTask() override {}

                int Open(TaskContext* ctx) override
                {
                        name_ += " " + ctx->Name();
                        SPDLOG_DEBUG("Task {}: open task ...", name_);
                        return 0;
                }

                int Process(TaskContext* ctx) override
                {
                        // input0
                        Datum proto_map_d = ctx->Inputs()[0].Value();
                        SPDLOG_DEBUG("Task {}: Consume Datum: {}", name_, proto_map_d);
                        auto proto_map_opt = proto_map_d.Consume<std::unordered_map<uint32_t, int>>();
                        if (!proto_map_opt)
                        {
                                SPDLOG_WARN(
                                    "Task {}: Consume() returns NULL, wait for "
                                    "input ...",
                                    name_);
                        }
                        // key is protocol, value is packet count of each protocol.
                        auto proto_map = *(proto_map_opt.value());
                        SPDLOG_DEBUG("Task {}: protoCountValue: {}", name_, proto_map);

                        // sidedata0
                        auto protoCountSum_d = ctx->GetInputSideData()[0];
                        SPDLOG_DEBUG(
                            "Task {}: Consume side data protoCountSum: {}",
                            name_,
                            protoCountSum_d);

                        auto protoCountSum_opt = protoCountSum_d.Consume<int>();
                        if (!protoCountSum_opt)
                        {
                                SPDLOG_WARN(
                                    "Task {}: Consume() returns NULL, wait for "
                                    "input ...",
                                    name_);
                        }

                        int protoCountSum = *(protoCountSum_opt.value());
                        SPDLOG_DEBUG("Task {}: protoCountSum: {}", name_, protoCountSum);

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
                        float_t score_max = 0.0;
                        float_t score = score_thresholds[2];
                        float_t min_ = (float_t) protoCountSum * ratioMin;
                        float_t max_ = (float_t) protoCountSum * ratioMax;
                        for (auto &&proto_stat : proto_map)
                        {
                                auto protoCountValue = proto_stat.second;

                                if (protoCountValue >= max_)
                                {
                                        score = score_thresholds[0];
                                }
                                else
                                {
                                        if (protoCountValue >= min_)
                                        {
                                                score = score_thresholds[1];
                                        }
                                }

                                SPDLOG_DEBUG("Task {}: after calculation: protocol: {}, score: {}", name_, proto_stat.first, score_max);

                                score_max = (score > score_max ? score : score_max);
                        }
                        SPDLOG_DEBUG("Task {}: after calculation: {}", name_, score_max);

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

        REGISTER(ProtocolStatsTask);

}   // namespace dni
