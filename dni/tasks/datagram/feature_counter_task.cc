#include <algorithm>
#include <unordered_map>
#include <vector>

#include "dni/framework/framework.h"
#include "dni/tasks/datagram/feature_counter_task.pb.h"
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
                Datum pcap_d = ctx->Inputs()[0].Value();
                SPDLOG_DEBUG("{}: Consume Datum: {}", name_, pcap_d);
                auto pcap_opt = pcap_d.Consume<
                    std::vector<std::unordered_map<std::string, uint32_t>>>();
                if (!pcap_opt)
                {
                        SPDLOG_CRITICAL("{}: invalid input", name_);
                        return -1;
                }
                auto pcap_packets = *(pcap_opt.value());
                SPDLOG_DEBUG("{}: pcap_packets: {}", name_, pcap_packets);

                SPDLOG_DEBUG("{}: features: {}", name_, options_.feature());

                std::vector<std::unordered_map<uint32_t, int>> ret;
                ret.resize(options_.feature().size());
                for (auto&& p : pcap_packets)
                {
                        for (int i = 0; i < options_.feature().size(); ++i)
                        {
                                auto val = p[options_.feature()[i]];
                                int& cnt = ret[i][val];
                                ++cnt;
                        }
                }

                SPDLOG_DEBUG("{}: ret: {}", name_, ret);

                for (int i = 0; i < options_.feature().size(); ++i)
                {
                        ctx->Outputs()[i].AddDatum(Datum(std::move(ret[i])));
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
