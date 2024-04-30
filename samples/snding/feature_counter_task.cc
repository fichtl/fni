#include <algorithm>
#include <vector>
#include <unordered_map>

#include "dni/framework/framework.h"
#include "spdlog/spdlog.h"

namespace dni {

        class FeatureCounterTask: public TaskBase {
        public:
                FeatureCounterTask(): name_("FeatureCounterTask") {}
                ~FeatureCounterTask() override {}

                int Open(TaskContext* ctx) override
                {
                        SPDLOG_DEBUG("Task {}: open task ...", name_);

                        return 0;
                }

                int Process(TaskContext* ctx) override
                {
                        // input
                        Datum pcap_d = ctx->Inputs()[0].Value();
                        SPDLOG_DEBUG("Task {}: Consume Datum: {}", name_, pcap_d);
                        auto pcap_opt = pcap_d.Consume<std::vector<std::unordered_map<std::string, uint32_t>>>();
                        if (!pcap_opt)
                        {
                                SPDLOG_WARN(
                                    "Task {}: Consume() returns NULL, wait for "
                                    "input ...",
                                    name_);
                        }
                        auto pcap_packets = *(pcap_opt.value());
                        SPDLOG_DEBUG("Task {}: pcap_packets: {}", name_, pcap_packets);

                        // sidedata0, features to count
                        auto features_d = ctx->GetInputSideData()[0];
                        SPDLOG_DEBUG(
                            "Task {}: Consume side data features: {}",
                            name_,
                            features_d);

                        auto features_opt = features_d.Consume<std::vector<std::string>>();
                        if (!features_opt)
                        {
                                SPDLOG_WARN(
                                    "Task {}: Consume() returns NULL, wait for "
                                    "input ...",
                                    name_);
                        }

                        auto features = *(features_opt.value());

                        SPDLOG_DEBUG(
                            "Task {}: features: {}", name_, features);


                        SPDLOG_DEBUG(
                            "Task {}: pcap_packets: {}", name_, pcap_packets.back()["SIP"]);

                        // count, need to spilit to {"SIP", "SPort", "DPort", "Protocol", "Length"}
                        std::vector<std::unordered_map<uint32_t, int>> ret; // map: key-number: count
                        ret.resize(features.size());
                        for (auto &&p : pcap_packets)
                        {
                                for (int i = 0; i < features.size(); ++i)
                                {
                                        auto val = p[features[i]];
                                        int& cnt = ret[i][val];
                                        ++cnt;
                                }
                        }

                        SPDLOG_DEBUG("count over");
                        SPDLOG_DEBUG("Task {}: ret: {}", name_, ret);

                        for (int i = 0; i < features.size(); ++i)
                        {
                                ctx->Outputs()[i].AddDatum(Datum(ret[i]));
                        }
                        

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

        REGISTER(FeatureCounterTask);

}   // namespace dni
