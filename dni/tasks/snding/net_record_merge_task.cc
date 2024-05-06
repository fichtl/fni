#include "dni/framework/framework.h"
#include "snding_defines.h"
#include "spdlog/spdlog.h"

namespace dni {

        class SndNetRecordMergeTask: public TaskBase {
        public:
                SndNetRecordMergeTask(): name_("SndNetRecordMergeTask") {}
                ~SndNetRecordMergeTask() override {}

                int Open(TaskContext* ctx) override
                {
                        SPDLOG_DEBUG("Task {}: open task ...", name_);

                        return 0;
                }

                int Process(TaskContext* ctx) override
                {
                        // input, all records of net, each record from one nic
                        size_t nic_size = ctx->Inputs().size();
                        std::vector<
                            std::unordered_map<std::string, snding::SIPBaseMergeStats>>
                            stats_in_all_net;
                        for (size_t i = 0; i < nic_size; i++)
                        {
                                Datum sip_base_merge_stats_d = ctx->Inputs()[i].Value();
                                SPDLOG_DEBUG(
                                    "Task {}: Consume sip_base_merge_stats: {}",
                                    name_,
                                    sip_base_merge_stats_d);
                                auto sip_base_merge_stats_opt =
                                    sip_base_merge_stats_d.Consume<std::unordered_map<
                                        std::string,
                                        snding::SIPBaseMergeStats>>();
                                if (!sip_base_merge_stats_opt)
                                {
                                        SPDLOG_WARN(
                                            "Task {}: Consume() sip_base_merge_stats "
                                            "returns NULL, wait for input ...",
                                            name_);
                                }
                                auto sip_base_merge_stats =
                                    *(sip_base_merge_stats_opt.value());
                                SPDLOG_DEBUG(
                                    "Task {}: val: {}", name_, sip_base_merge_stats);

                                stats_in_all_net.emplace_back(
                                    std::move(sip_base_merge_stats));
                        }
                        // match
                        // key: srcip-cidr:dstip/32, format is
                        // uint32/len:uint32/len
                        std::unordered_map<std::string, std::unordered_set<std::string>>
                            hosts_in_attack_link;
                        for (auto&& nic_stats : stats_in_all_net)
                        {
                                // stat is a snding::SIPBaseMergeStats
                                for (auto&& stat : nic_stats)
                                {
                                        for (auto&& dip : stat.second.dstIP.value)
                                        {
                                                // downstream node will use
                                                // "RANDOM-dstip" or "srcip-dstip"
                                                // to match attack link. all
                                                // "RANDOM-dstip" matched host as
                                                // an unique attack link.
                                                auto key = stat.first + ":" +
                                                           std::to_string(dip) + "/32";
                                                SPDLOG_DEBUG(
                                                    "key: {}, hostNicSign: {}",
                                                    key,
                                                    stat.second.hostNicSign);
                                                auto& attack_link =
                                                    hosts_in_attack_link[key];
                                                attack_link.insert(
                                                    stat.second.hostNicSign);
                                        }
                                }
                        }

                        SPDLOG_DEBUG(
                            "Task {}: after calculation: {}",
                            name_,
                            hosts_in_attack_link);

                        ctx->Outputs()[0].AddDatum(
                            Datum(std::move(hosts_in_attack_link)));

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

        REGISTER(SndNetRecordMergeTask);

}   // namespace dni
