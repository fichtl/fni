#include <unordered_map>

#include "dni/framework/framework.h"
#include "snding_defines.h"
#include "spdlog/spdlog.h"

// This task do not use backtrace infomation, all attack path will receive the dms rules.
// TODO: add filter for unrelated A-nodes.
namespace dni {

        class SndGenDMSRulesTask: public TaskBase {
        public:
                SndGenDMSRulesTask(): name_("SndGenDMSRulesTask") {}
                ~SndGenDMSRulesTask() override {}

                int Open(TaskContext* ctx) override
                {
                        SPDLOG_DEBUG("Task {}: open task ...", name_);

                        return 0;
                }

                int Process(TaskContext* ctx) override
                {
                        // dms general rules
                        // key: host#nic-name
                        std::unordered_map<std::string, std::vector<snding::DMSRule>>
                            nic_rules;

                        auto nsize = ctx->Inputs().size();
                        SPDLOG_DEBUG("Task {}: input size: {}", name_, nsize);
                        for (size_t i = 0; i < nsize; i++)
                        {
                                // input in even index
                                // each map is from sip based merge result from one NIC.
                                Datum sip_base_merge_stats_d = ctx->Inputs()[i].Value();
                                SPDLOG_DEBUG(
                                    "Task {}: Consume sip_base_merge_stats: {}",
                                    name_,
                                    sip_base_merge_stats_d);
                                auto sip_base_merge_stats_opt =
                                    sip_base_merge_stats_d.Consume<std::unordered_map<
                                        std::string, snding::SIPBaseMergeStats>>();
                                if (!sip_base_merge_stats_opt)
                                {
                                        SPDLOG_WARN(
                                            "Task {}, index-{}: Consume() "
                                            "sip_base_merge_stats returns NULL, wait for "
                                            "input ...",
                                            name_, i);
                                }
                                auto sip_base_merge_stats =
                                    *(sip_base_merge_stats_opt.value());
                                SPDLOG_DEBUG(
                                    "Task {}: val: {}", name_, sip_base_merge_stats);

                                // input in odd index is netdev calc result,
                                // each inner vector includes:
                                // inMbps Value, inMbps score, inKpps Value, inKpps score
                                i++;
                                Datum netdevs_d = ctx->Inputs()[i].Value();
                                SPDLOG_DEBUG(
                                    "Task {}: Consume netdevs: {}", name_, netdevs_d);
                                auto netdevs_opt =
                                    netdevs_d.Consume<std::vector<double_t>>();
                                if (!netdevs_opt)
                                {
                                        SPDLOG_WARN(
                                            "Task {}, index-{}: Consume() net_devs "
                                            "returns "
                                            "NULL, wait for "
                                            "input ...",
                                            name_, i);
                                }
                                auto netdevs = *(netdevs_opt.value());
                                SPDLOG_DEBUG("Task {}: val: {}", name_, netdevs);

                                // generate dms rules
                                for (auto&& stat : sip_base_merge_stats)
                                {
                                        auto& rules = nic_rules[stat.second.hostNicSign];

                                        std::vector<snding::DMSRule> dms_rules =
                                            generate_dms_rules(stat.second, netdevs);

                                        rules.insert(
                                            rules.end(),
                                            dms_rules.begin(),
                                            dms_rules.end());
                                }
                        }

                        SPDLOG_DEBUG("Task {}: after calculation: {}", name_, nic_rules);

                        ctx->Outputs()[0].AddDatum(Datum(std::move(nic_rules)));

                        return 0;
                }

                int Close(TaskContext* ctx) override
                {
                        SPDLOG_DEBUG("Task {}: closing ...", name_);

                        return 0;
                }

        private:
                std::string name_;

                std::vector<snding::DMSRule> generate_dms_rules(
                    const snding::SIPBaseMergeStats& stat,
                    const std::vector<double_t>& netdev);

                void calc_limit_rule(
                    std::string& limitMode, uint64_t& limitMaxValue,
                    const std::vector<double_t>& netdev);
        };

        std::vector<snding::DMSRule> SndGenDMSRulesTask::generate_dms_rules(
            const snding::SIPBaseMergeStats& stat, const std::vector<double_t>& netdev)
        {
                std::string action = "drop";
                std::string limitMode;
                uint64_t limitMaxValue = 0;

                SPDLOG_DEBUG(
                    "Task {}: {}-{}-{}", name_, (stat.isSrcIPRandom ? 0 : 1),
                    (stat.srcPort.stat_type == "centralize" ? 1 : 0),
                    (stat.dstPort.stat_type == "centralize" ? 1 : 0));

                // quick action judge
                if ((!stat.isSrcIPRandom) || (stat.srcPort.stat_type == "centralize") ||
                    (stat.dstPort.stat_type == "centralize"))
                {
                        SPDLOG_DEBUG("Task {}: action is 'drop'", name_);
                        action = "drop";
                }
                else
                {
                        SPDLOG_DEBUG("Task {}: action is 'limit'", name_);
                        action = "limit";
                        calc_limit_rule(limitMode, limitMaxValue, netdev);
                }

                std::vector<snding::DMSRule> gen_rules;
                int rules_num = 1;

                std::vector<int> sPorts;
                if (stat.srcPort.stat_type == "centralize")
                {
                        sPorts.insert(
                            sPorts.end(),
                            stat.srcPort.value.begin(),
                            stat.srcPort.value.end());
                }
                rules_num *= (sPorts.size() == 0 ? 1 : sPorts.size());
                if (sPorts.size() == 0)
                {
                        sPorts.push_back(-1);
                }

                std::vector<int> dPorts;
                if (stat.dstPort.stat_type == "centralize")
                {
                        dPorts.insert(
                            dPorts.end(),
                            stat.dstPort.value.begin(),
                            stat.dstPort.value.end());
                }
                rules_num *= (dPorts.size() == 0 ? 1 : dPorts.size());
                if (dPorts.size() == 0)
                {
                        dPorts.push_back(-1);
                }

                std::vector<int> protocols;
                for (auto&& proto : stat.protocol.stat_types)
                {
                        protocols.emplace_back(proto.first);
                }
                rules_num *= (protocols.size() == 0 ? 1 : protocols.size());
                if (protocols.size() == 0)
                {
                        protocols.push_back(-1);
                }

                gen_rules.resize(rules_num);
                int index = -1;
                for (size_t i = 0; i < sPorts.size(); i++)
                {
                        for (size_t j = 0; j < dPorts.size(); j++)
                        {
                                for (size_t k = 0; k < protocols.size(); k++)
                                {
                                        index++;
                                        gen_rules[index].hostNicSign =
                                            std::move(stat.hostNicSign);
                                        gen_rules[index].srcIP = {
                                            stat.srcIP.ip, stat.srcIP.len};
                                        // gen_rules[index].dstIP = {
                                        //     stat.dstIP.ip, stat.dstIP.len};
                                        gen_rules[index].sPort = sPorts[i];
                                        gen_rules[index].dPort = dPorts[j];
                                        gen_rules[index].protocol = protocols[k];

                                        gen_rules[index].action = action;
                                        gen_rules[index].limitMode = limitMode;
                                        gen_rules[index].limitMaxValue = limitMaxValue;
                                }
                        }
                }

                return std::move(gen_rules);
        }

        void SndGenDMSRulesTask::calc_limit_rule(
            std::string& limitMode, uint64_t& limitMaxValue,
            const std::vector<double_t>& netdev)
        {
                // bps Value, bps score, pps Value, pps score
                if ((netdev[1] - netdev[3]) > 1e-6)
                {
                        // choose limit mode bps
                        limitMode = "bps";
                        limitMaxValue = (uint64_t) (netdev[0] / 2);
                }
                else
                {
                        // choose limit mode pps
                        limitMode = "pps";
                        limitMaxValue = (uint64_t) (netdev[2] / 2);
                }
        }

        REGISTER(SndGenDMSRulesTask);

}   // namespace dni
