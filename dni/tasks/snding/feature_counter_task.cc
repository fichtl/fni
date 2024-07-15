#include <algorithm>
#include <unordered_map>
#include <vector>

#include "dni/framework/framework.h"
#include "dni/tasks/snding/feature_counter_task.pb.h"
#include "spdlog/spdlog.h"

namespace dni {

class FeatureCounterTask: public TaskBase {
public:
        FeatureCounterTask(): name_("FeatureCounterTask") {}
        ~FeatureCounterTask() override {}

        int Open(TaskContext* ctx) override
        {
                name_ += "(" + ctx->Name() + ")";
                SPDLOG_DEBUG("{}: open task ...", name_);

                options_ = ctx->Options<FeatureCounterTaskOptions>();

                return 0;
        }

        int Process(TaskContext* ctx) override
        {
                // input
                // 0:SIP, 1:SPort, 2:DPort, 3:Protocol, 4:Length, 5:DIP
                Datum pcap_d = ctx->Inputs()[0].Value();
                SPDLOG_DEBUG("{}: Consume Datum: {}", name_, pcap_d);
                auto pcap_opt = pcap_d.Consume<std::vector<std::vector<uint32_t>>*>();
                if (!pcap_opt)
                {
                        SPDLOG_CRITICAL("{}: invalid input", name_);
                        return -1;
                }
                auto pcap_packets = *(pcap_opt.value());
                SPDLOG_TRACE("{}: pcap_packets: {}", name_, pcap_packets);

                // "SIP","SPort","DPort","Protocol","Length"
                SPDLOG_DEBUG("{}: features: {}", name_, options_.feature());
                // same order with above
                std::vector<std::unordered_map<uint32_t, int>> ret;
                auto pkts_size = pcap_packets->size();
                auto features = options_.feature();
                auto feat_size = features.size();
                ret.resize(feat_size);
                for (size_t i = 0; i < pkts_size; i++)
                {
                        auto& p = pcap_packets->at(i);
                        for (size_t j = 0; j < feat_size; j++)
                        {
                                auto val = p[j];
                                int& cnt = ret[j][val];
                                ++cnt;
                        }
                }

                SPDLOG_TRACE("{}: ret: {}", name_, ret);

                // for (auto& feat : options_.feature())
                // {
                //         ctx->Outputs().Tag(feat).AddDatum(Datum(std::move(ret[feat])));
                // }
                for (size_t i = 0; i < feat_size; i++)
                {
                        ctx->Outputs()
                            .Tag(features[i])
                            .AddDatum(Datum(std::move(ret[i])));
                }

                return 0;
        }

        int Close(TaskContext* ctx) override
        {
                SPDLOG_DEBUG("{}: closing ...", name_);

                return 0;
        }

private:
        std::string name_;

        FeatureCounterTaskOptions options_;
};

REGISTER(FeatureCounterTask);

}   // namespace dni
