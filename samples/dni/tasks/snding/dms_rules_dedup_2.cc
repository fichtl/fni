#include <chrono>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "dni/framework/framework.h"
#include "dni/tasks/snding/snding_defines.h"
#include "spdlog/spdlog.h"

using namespace std::string_literals;

void inject_after(dni::Graph* g, int after, int n, int interval)
{
        std::this_thread::sleep_for(std::chrono::milliseconds(after));

        /*

                                                                          (5.6.7.9/32)
                                                          |-------------> target2:eno4
                     SIP1                                 |
        attacker0:eno1 ~ attacker256:eno1----------> A10-1:eno2 --------> target1:eno3
                1.2.3.0/24                          10.10.10.10            (5.6.7.8/32)

                                                         /|\
                                                          |
                                                          |
                                                          |
                      SIP2
        attacker256:eno1 ~ attacker511:eno1--------> A10-2:eno2
                A1.A2.A3.0/24                       20.20.20.20


        */

        // 2. input stream

        srand(time(NULL));

        // SIP1 ----> A10-1 ----> target1
        std::vector<std::unordered_map<std::string, uint32_t>> parsed_packets_A10_1;
        std::vector<std::unordered_map<std::string, uint32_t>> parsed_packets_A10_2;
        std::vector<std::unordered_map<std::string, uint32_t>> parsed_packets_Target_1;
        std::vector<std::unordered_map<std::string, uint32_t>> parsed_packets_Target_2;
        for (size_t i = 0; i < 5000; i++)
        {
                std::unordered_map<std::string, uint32_t> p1;

                p1["SIP"] = 0x01020300 + rand() % 256;   // 0~255
                p1["DIP"] = 0x05060708;
                p1["SPort"] = rand() % 65536;   // 1~65535
                p1["DPort"] = 5678;
                p1["Protocol"] = 17;
                p1["Length"] = rand() % 1300 + 100;   // 100~1400
                if (i % 2 == 0)
                        parsed_packets_A10_1.emplace_back(p1);
                parsed_packets_Target_1.emplace_back(p1);
        }

        // SIP2 ----> A10-2 ----> A10-1 ----> target1
        for (size_t i = 0; i < (10000 - 5000); i++)
        {
                std::unordered_map<std::string, uint32_t> p1;

                p1["SIP"] = 0xa1a2a300 + rand() % 256;   // 0~255
                p1["DIP"] = 0x05060708;
                p1["SPort"] = rand() % 65536;   // 1~65535
                p1["DPort"] = (rand() % 2 == 0 ? 22222 : 33333);
                p1["Protocol"] = (rand() % 2 == 0 ? 17 : 6);
                p1["Length"] = rand() % 1300 + 100;   // 100~1400
                parsed_packets_A10_2.emplace_back(p1);
                if (i % 2 == 0)
                        parsed_packets_A10_1.emplace_back(p1);
                parsed_packets_Target_1.emplace_back(p1);
        }

        // SIP1 ----> A10-1 ----> target2
        for (size_t i = 0; i < 5000; i++)
        {
                std::unordered_map<std::string, uint32_t> p1;

                p1["SIP"] = 0x01020300 + rand() % 256;   // 0~255
                p1["DIP"] = 0x05060709;
                p1["SPort"] = rand() % 65536;   // 1~65535
                p1["DPort"] = 5678;
                p1["Protocol"] = 17;
                p1["Length"] = rand() % 1300 + 100;   // 100~1400
                if (i % 2 == 0)
                        parsed_packets_A10_1.emplace_back(p1);
                parsed_packets_Target_2.emplace_back(p1);
        }

        // SIP2 ----> A10-2 ----> A10-1 ----> target2
        for (size_t i = 0; i < (10000 - 5000); i++)
        {
                std::unordered_map<std::string, uint32_t> p1;

                p1["SIP"] = 0xa1a2a300 + rand() % 256;   // 0~255
                p1["DIP"] = 0x05060709;
                p1["SPort"] = rand() % 65536;   // 1~65535
                p1["DPort"] = (rand() % 2 == 0 ? 22222 : 33333);
                p1["Protocol"] = (rand() % 2 == 0 ? 17 : 6);
                p1["Length"] = rand() % 1300 + 100;   // 100~1400
                parsed_packets_A10_2.emplace_back(p1);
                if (i % 2 == 0)
                        parsed_packets_A10_1.emplace_back(p1);
                parsed_packets_Target_2.emplace_back(p1);
        }

        //
        std::unordered_map<uint32_t, int> ip_map;
        for (auto&& packet : parsed_packets_A10_1)
        {
                ip_map[packet["SIP"]] = 0;   // do not care value of map
        }
        dni::snding::AttackerIPMergeResult cidr_merged_sip_A10_1;
        cidr_merged_sip_A10_1.attackerIPs = std::move(dni::cidr_merge(ip_map));

        g->AddDatumToInputStream("parsed_packets_1", dni::Datum(parsed_packets_A10_1));
        g->AddDatumToInputStream("cidr_merged_sip_1", dni::Datum(cidr_merged_sip_A10_1));
        g->AddDatumToInputStream("host_nic_name1", dni::Datum("A10-1#eno2"s));

        std::vector<double_t> netdevs;
        netdevs.push_back(4.00 * 1e9);
        netdevs.push_back(0.6);
        netdevs.push_back(16.00 * 1e6);
        netdevs.push_back(1.00);
        g->AddDatumToInputStream("netdevs_1", dni::Datum(netdevs));

        //
        ip_map.clear();
        for (auto&& packet : parsed_packets_A10_2)
        {
                ip_map[packet["SIP"]] = 0;   // do not care value of map
        }
        dni::snding::AttackerIPMergeResult cidr_merged_sip_A10_2;
        cidr_merged_sip_A10_2.attackerIPs = std::move(dni::cidr_merge(ip_map));

        g->AddDatumToInputStream("parsed_packets_2", dni::Datum(parsed_packets_A10_2));
        g->AddDatumToInputStream("cidr_merged_sip_2", dni::Datum(cidr_merged_sip_A10_2));
        g->AddDatumToInputStream("host_nic_name2", dni::Datum("A10-2#eno2"s));

        g->AddDatumToInputStream("netdevs_2", dni::Datum(netdevs));

        //
        ip_map.clear();
        for (auto&& packet : parsed_packets_Target_1)
        {
                ip_map[packet["SIP"]] = 0;   // do not care value of map
        }
        dni::snding::AttackerIPMergeResult cidr_merged_sip_Target_1;
        cidr_merged_sip_Target_1.attackerIPs = std::move(dni::cidr_merge(ip_map));

        g->AddDatumToInputStream("parsed_packets_3", dni::Datum(parsed_packets_Target_1));
        g->AddDatumToInputStream(
            "cidr_merged_sip_3", dni::Datum(cidr_merged_sip_Target_1));
        g->AddDatumToInputStream("host_nic_name3", dni::Datum("target1#eno3"s));

        g->AddDatumToInputStream("netdevs_3", dni::Datum(netdevs));

        //
        ip_map.clear();
        for (auto&& packet : parsed_packets_Target_2)
        {
                ip_map[packet["SIP"]] = 0;   // do not care value of map
        }
        dni::snding::AttackerIPMergeResult cidr_merged_sip_Target_2;
        cidr_merged_sip_Target_2.attackerIPs = std::move(dni::cidr_merge(ip_map));

        g->AddDatumToInputStream("parsed_packets_4", dni::Datum(parsed_packets_Target_2));
        g->AddDatumToInputStream(
            "cidr_merged_sip_4", dni::Datum(cidr_merged_sip_Target_2));
        g->AddDatumToInputStream("host_nic_name4", dni::Datum("target2#eno4"s));

        g->AddDatumToInputStream("netdevs_4", dni::Datum(netdevs));
}

int main()
{
        spdlog::set_level(spdlog::level::trace);

        const std::string& proto =
            "samples/dni/tasks/snding/testdata/dms_rules_dedup_2.pbtxt";
        auto gc = dni::LoadTextprotoFile(proto);
        if (!gc)
        {
                spdlog::error("invalid pbtxt config: {}", proto);
                return -1;
        }
        spdlog::debug(
            "GraphConfig: {}, node size: {}", gc.value().type(), gc.value().node_size());

        dni::Graph* g = new dni::Graph(gc.value());

        std::string out = "dms_rules";

        spdlog::debug("Create ObserveOutputStream: {}", out);
        g->ObserveOutputStream(out);

        g->PrepareForRun();

        inject_after(g, 0, 1, 0);

        g->RunOnce();

        g->Wait();

        auto ret = g->GetResult<
            std::unordered_map<std::string, std::vector<dni::snding::DMSRule>>>(out);
        spdlog::info("Gout {} result is: {}", out, ret.size());
        for (auto&& pair : ret)
        {
                spdlog::info(
                    "host-nicname:{}\t , dms-rule size: {}\n", pair.first,
                    pair.second.size());
                for (auto&& rule : pair.second)
                {
                        spdlog::info("{}\n", rule);
                }
                spdlog::info("-----------------------------------\n");
        }

        g->Finish();

        spdlog::info("main over");

        return 0;
}
