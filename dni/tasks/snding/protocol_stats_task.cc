#include <unordered_map>
#include <unordered_set>

#include "dni/framework/framework.h"
#include "dni/tasks/snding/protocol_stats_task.pb.h"
#include "spdlog/spdlog.h"

namespace dni {

class SndProtocolStatsTask: public TaskBase {
public:
        SndProtocolStatsTask(): name_("SndProtocolStatsTask") {}
        ~SndProtocolStatsTask() override {}

        int Open(TaskContext* ctx) override
        {
                name_ += "(" + ctx->Name() + ")";
                SPDLOG_DEBUG("{}: open task ...", name_);

                options_ = ctx->Options<SndProtocolStatsTaskOptions>();
                protoCountSum_ = options_.protocountsum();
                ratioMin_ = options_.ratiomin();
                ratioMax_ = options_.ratiomax();
                if (options_.score_thresholds_size() != 3)
                {
                        SPDLOG_ERROR(
                            "{}: invalid score thresholds size, expect 3, get {}",
                            name_,
                            options_.score_thresholds_size());
                        return -1;
                }
                SPDLOG_DEBUG(
                    "{}: protoCountSum_: {}, ratioMin: {}, ratioMax: {}, "
                    "score_thresholds: {}",
                    name_, protoCountSum_, ratioMin_, ratioMax_,
                    options_.score_thresholds_size());

                return 0;
        }

        int Process(TaskContext* ctx) override
        {
                // input0
                Datum proto_map_d = ctx->Inputs()[0].Value();
                SPDLOG_DEBUG("{}: Consume proto_map: {}", name_, proto_map_d);
                auto proto_map_opt =
                    proto_map_d.Consume<std::unordered_map<uint32_t, int>>();
                if (!proto_map_opt)
                {
                        SPDLOG_CRITICAL("{}: invalid input proto_map", name_);
                        return -1;
                }
                // key is protocol, value is packet count of each protocol.
                auto proto_map = *(proto_map_opt.value());
                SPDLOG_DEBUG("{}: protoCountValue: {}", name_, proto_map);

                // score
                double_t score_max = 0.0;
                double_t score = options_.score_thresholds()[2];
                double_t min_ = (double_t) protoCountSum_ * ratioMin_;
                double_t max_ = (double_t) protoCountSum_ * ratioMax_;
                for (auto&& proto_stat : proto_map)
                {
                        auto protoCountValue = proto_stat.second;

                        if (protoCountValue >= max_)
                        {
                                score = options_.score_thresholds()[0];
                        }
                        else
                        {
                                if (protoCountValue >= min_)
                                {
                                        score = options_.score_thresholds()[1];
                                }
                        }

                        SPDLOG_DEBUG(
                            "{}: after calculation: protocol: {}, "
                            "score: {}",
                            name_,
                            proto_stat.first,
                            score_max);

                        score_max = (score > score_max ? score : score_max);
                }
                SPDLOG_DEBUG("{}: after calculation: {}", name_, score_max);

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

        SndProtocolStatsTaskOptions options_;
        int protoCountSum_;
        double ratioMin_;
        double ratioMax_;
};

REGISTER(SndProtocolStatsTask);

}   // namespace dni
