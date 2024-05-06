#include <iostream>
#include <vector>

#include "dni/tasks/snding/snding_defines.h"
#include "spdlog/spdlog.h"

std::vector<dni::snding::DMSRule> generate_dms_rules(const dni::snding::SIPBaseMergeStats& stat)
{
        std::string action = "drop";
        // action judge
        // if (stat.srcIP.stat_type != "centralize" && stat.srcPort.stat_type !=
        // "centralize" && stat.dstPort.stat_type != "centralize")
        // {
        //         SPDLOG_DEBUG("Task {}: dms action will be 'limit'", name_);
        //         action = "limit";
        // }

        std::vector<dni::snding::DMSRule> gen_rules;
        int rules_num = 1;

        std::vector<int> sPorts;
        if (stat.srcPort.stat_type == "centralize")
        {
                sPorts.insert(
                    sPorts.end(), stat.srcPort.value.begin(), stat.srcPort.value.end());
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
                    dPorts.end(), stat.dstPort.value.begin(), stat.dstPort.value.end());
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

        spdlog::info("rules_num {}\n", rules_num);

        gen_rules.resize(rules_num);
        int index = -1;
        for (size_t i = 0; i < sPorts.size(); i++)
        {
                for (size_t j = 0; j < dPorts.size(); j++)
                {
                        for (size_t k = 0; k < protocols.size(); k++)
                        {
                                index++;
                                gen_rules[index].srcIP = {stat.srcIP.ip, stat.srcIP.len};
                                // gen_rules[index].dstIP = {stat.dstIP.ip, stat.dstIP.len};
                                gen_rules[index].sPort = sPorts[i];
                                gen_rules[index].dPort = dPorts[j];
                                gen_rules[index].protocol = protocols[k];
                        }
                }
        }

        return std::move(gen_rules);
}

dni::snding::SIPBaseMergeStats gen1()
{
        dni::snding::SIPBaseMergeStats stats;
        stats.srcIP = {0x01020300, 24};
        stats.dstIP.value.insert(0x05060708);

        stats.srcPort.stat_type = "centralize";
        stats.srcPort.value = {1000, 2000, 3000, 4000};

        stats.dstPort.stat_type = "centralize";
        stats.dstPort.value = {500, 600, 700};

        stats.protocol.stat_types[6] = "rapid";
        stats.protocol.stat_types[17] = "rapid";

        return std::move(stats);
}

dni::snding::SIPBaseMergeStats gen2()
{
        dni::snding::SIPBaseMergeStats stats;
        stats.srcIP = {0x01020300, 24};
        stats.dstIP.value.insert(0x05060708);

        stats.srcPort.stat_type = "centralize";
        stats.srcPort.value = {1000, 2000, 3000, 4000};

        stats.dstPort.stat_type = "centralize";
        stats.dstPort.value = {500, 600, 700};

        return std::move(stats);
}

dni::snding::SIPBaseMergeStats gen3()
{
        dni::snding::SIPBaseMergeStats stats;
        stats.srcIP = {0x01020300, 24};
        stats.dstIP.value.insert(0x05060708);

        stats.srcPort.stat_type = "centralize";
        stats.srcPort.value = {1000, 2000, 3000, 4000};

        return std::move(stats);
}

dni::snding::SIPBaseMergeStats gen4()
{
        dni::snding::SIPBaseMergeStats stats;
        stats.srcIP = {0x01020300, 24};
        stats.dstIP.value.insert(0x05060708);

        return std::move(stats);
}

int main()
{
        {
                auto stats = gen1();
                auto ret = generate_dms_rules(stats);
                spdlog::info("dms-rule size: {}\n", ret.size());
                for (auto&& rule : ret)
                {
                        spdlog::info("{}\n", rule);
                }
        }

        spdlog::info("\n\n\n");

        {
                auto stats = gen2();
                auto ret = generate_dms_rules(stats);
                spdlog::info("dms-rule size: {}\n", ret.size());
                for (auto&& rule : ret)
                {
                        spdlog::info("{}\n", rule);
                }
        }

        spdlog::info("\n\n\n");

        {
                auto stats = gen3();
                auto ret = generate_dms_rules(stats);
                spdlog::info("dms-rule size: {}\n", ret.size());
                for (auto&& rule : ret)
                {
                        spdlog::info("{}\n", rule);
                }
        }

        spdlog::info("\n\n\n");

        {
                auto stats = gen4();
                auto ret = generate_dms_rules(stats);
                spdlog::info("dms-rule size: {}\n", ret.size());
                for (auto&& rule : ret)
                {
                        spdlog::info("{}\n", rule);
                }
        }
}