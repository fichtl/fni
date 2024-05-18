#include <unordered_map>
#include <vector>

#include "dni/framework/framework.h"
#include "snding_defines.h"
#include "spdlog/spdlog.h"

// This task do not use backtrace infomation, all attack path will receive the dms rules.
// TODO: add filter for unrelated A-nodes.
namespace dni {

class SndGenDeDupDMSRulesTask: public TaskBase {
public:
        SndGenDeDupDMSRulesTask(): name_("SndGenDeDupDMSRulesTask") {}
        ~SndGenDeDupDMSRulesTask() override {}

        int Open(TaskContext* ctx) override
        {
                SPDLOG_DEBUG("Task {}: open task ...", name_);

                return 0;
        }

        int Process(TaskContext* ctx) override
        {
                // dms general rules
                // key: host#nic-name
                snding::NICDMSRulesMap nic_rules;

                auto nsize = ctx->Inputs().size();
                SPDLOG_DEBUG("{}: input size: {}", name_, nsize);
                for (size_t i = 0; i < nsize / 2; i++)
                {
                        // CIDR is a map comes from sip based merge result
                        Datum cidr_d = ctx->Inputs().Get("CIDR", i).Value();
                        SPDLOG_DEBUG("{}: {}: Consume cidr: {}", name_, i, cidr_d);
                        auto cidr_opt = cidr_d.Consume<std::unordered_map<
                            std::string, std::unordered_map<std::string, std::string>>>();
                        if (!cidr_opt)
                        {
                                SPDLOG_CRITICAL("{}: {}: invalid cidr", name_, i);
                                return -1;
                        }
                        auto cidr = *(cidr_opt.value());
                        SPDLOG_DEBUG("{}: val: {}", name_, cidr);

                        // NETDEV is netdev calc result, each includes: inMbps Value,
                        // inMbps score, inKpps Value, inKpps score
                        Datum netdevs_d = ctx->Inputs().Get("NETDEV", i).Value();
                        SPDLOG_DEBUG("{}: {}: Consume netdevs: {}", name_, i, netdevs_d);
                        auto netdevs_opt = netdevs_d.Consume<std::vector<double_t>>();
                        if (!netdevs_opt)
                        {
                                SPDLOG_CRITICAL("{}: {}: invalid netdevs", name_, i);
                                return -1;
                        }
                        auto netdevs = *(netdevs_opt.value());
                        SPDLOG_DEBUG("{}: val: {}", name_, netdevs);

                        // generate dms rules
                        for (auto&& stat : cidr)
                        {
                                auto& rules = nic_rules[stat.second["hostNicSign"]];

                                snding::DMSRule dms_rule =
                                    generate_dms_rule(stat.second, netdevs);

                                rules.emplace_back(dms_rule);
                        }
                }

                SPDLOG_DEBUG("{}: after calculation: {}", name_, nic_rules);

                ctx->Outputs()[0].AddDatum(Datum(std::move(nic_rules)));

                return 0;
        }

        int Close(TaskContext* ctx) override
        {
                SPDLOG_DEBUG("{}: closing ...", name_);

                return 0;
        }

private:
        std::string name_;

        snding::DMSRule generate_dms_rule(
            std::unordered_map<std::string, std::string>& stat,
            std::vector<double_t>& netdev);

        void calc_limit_rule(
            std::string& limitMode, uint64_t& limitMaxValue,
            std::vector<double_t>& netdev);
};

snding::DMSRule SndGenDeDupDMSRulesTask::generate_dms_rule(
    std::unordered_map<std::string, std::string>& stat, std::vector<double_t>& netdev)
{
        std::string action = "drop";
        std::string limitMode;
        uint64_t limitMaxValue = 0;

        SPDLOG_DEBUG("{}: {}-{}-{}", name_, stat["SIP"], stat["SPort"], stat["DPort"]);

        // quick action judge
        if ((stat["SIP"] != "RANDOM") || (stat["SPort"] != "-1") ||
            (stat["DPort"] != "-1"))
        {
                SPDLOG_DEBUG("{}: action is 'drop'", name_);
                action = "drop";
        }
        else
        {
                SPDLOG_DEBUG("{}: action is 'limit'", name_);
                action = "limit";
                calc_limit_rule(limitMode, limitMaxValue, netdev);
        }

        snding::DMSRule gen_rule;

        gen_rule.hostNicSign = std::move(stat["hostNicSign"]);

        if (stat["SIP"] == "RANDOM")
        {
                gen_rule.srcIP = {0, -1};
        }
        else
        {
                int idx = stat["SIP"].find("/");
                auto ip = std::stoul(stat["SIP"].substr(0, idx));
                auto len = std::stoi(stat["SIP"].substr(idx + 1));
                gen_rule.srcIP = {static_cast<uint32_t>(ip), len};
        }

        auto ip = std::stoul(stat["DIP"]);
        gen_rule.dstIP = {static_cast<uint32_t>(ip), 32};

        if (stat["SPort"] != "-1")
        {
                gen_rule.sPort = std::stoi(stat["SPort"]);
        }
        else
        {
                gen_rule.sPort = -1;
        }

        if (stat["DPort"] != "-1")
        {
                gen_rule.dPort = std::stoi(stat["DPort"]);
        }
        else
        {
                gen_rule.dPort = -1;
        }

        gen_rule.protocol = std::stoi(stat["Protocol"]);

        gen_rule.action = action;
        gen_rule.limitMode = limitMode;
        gen_rule.limitMaxValue = limitMaxValue;

        return std::move(gen_rule);
}

void SndGenDeDupDMSRulesTask::calc_limit_rule(
    std::string& limitMode, uint64_t& limitMaxValue, std::vector<double_t>& netdev)
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

REGISTER(SndGenDeDupDMSRulesTask);

}   // namespace dni
