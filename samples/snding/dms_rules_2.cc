#include <chrono>
#include <thread>

#include "dni/framework/framework.h"
#include "dni/tasks/snding/snding_defines.h"
#include "spdlog/spdlog.h"

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

        // 1. input sidedata
        double_t number_ratio_min = 0.1;
        double_t number_ratio_max = 0.6;
        std::vector<std::string> number_stat_type = {
            "centralize", "regular", "random", "void"};

        double_t protocol_ratio_min = 0.1;
        double_t protocol_ratio_max = 0.6;
        std::vector<std::string> protocol_stat_type = {"flood", "rapid", "void"};

        g->AddDatumToInputSideData("number_ratio_min", dni::Datum(number_ratio_min));
        g->AddDatumToInputSideData("number_ratio_max", dni::Datum(number_ratio_max));
        g->AddDatumToInputSideData("number_stat_type", dni::Datum(number_stat_type));

        g->AddDatumToInputSideData("protocol_ratio_min", dni::Datum(protocol_ratio_min));
        g->AddDatumToInputSideData("protocol_ratio_max", dni::Datum(protocol_ratio_max));
        g->AddDatumToInputSideData("protocol_stat_type", dni::Datum(protocol_stat_type));

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
                if (i % 2 == 0) parsed_packets_A10_1.emplace_back(p1);
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
                if (i % 2 == 0)  parsed_packets_A10_1.emplace_back(p1);
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
                if (i % 2 == 0)  parsed_packets_A10_1.emplace_back(p1);
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
                if (i % 2 == 0)  parsed_packets_A10_1.emplace_back(p1);
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
        g->AddDatumToInputStream("host_nic_name1", dni::Datum(std::string("A10-1#eno2")));

        std::vector<double_t> netdevs;
        netdevs.push_back(4.00*1e9);
        netdevs.push_back(0.6);
        netdevs.push_back(16.00*1e6);
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
        g->AddDatumToInputStream("host_nic_name2", dni::Datum(std::string("A10-2#eno2")));

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
        g->AddDatumToInputStream("cidr_merged_sip_3", dni::Datum(cidr_merged_sip_Target_1));
        g->AddDatumToInputStream("host_nic_name3", dni::Datum(std::string("target1#eno3")));

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
        g->AddDatumToInputStream("cidr_merged_sip_4", dni::Datum(cidr_merged_sip_Target_2));
        g->AddDatumToInputStream("host_nic_name4", dni::Datum(std::string("target2#eno4")));

        g->AddDatumToInputStream("netdevs_4", dni::Datum(netdevs));

}

int main()
{
        spdlog::set_level(spdlog::level::trace);

        const std::string& proto = R"pb(
                type: "SndDMSRules"

                input_side_data: "GSD_NumberRatioMin:0:number_ratio_min"
                input_side_data: "GSD_NumberRatioMax:0:number_ratio_max"
                input_side_data: "GSD_NumberStatType:0:number_stat_type"
                input_side_data: "GSD_ProtocolRatioMin:0:protocol_ratio_min"
                input_side_data: "GSD_ProtocolRatioMax:0:protocol_ratio_max"
                input_side_data: "GSD_ProtocolStatType:0:protocol_stat_type"

                input_stream: "GIN_ParsedPackets_1:0:parsed_packets_1"
                input_stream: "GIN_CidrMergedSIP_1:0:cidr_merged_sip_1"
                input_stream: "GIN_HostNicName1:0:host_nic_name1"
                input_stream: "GIN_Netdevs_1:0:netdevs_1"

                input_stream: "GIN_ParsedPackets_2:0:parsed_packets_2"
                input_stream: "GIN_CidrMergedSIP_2:0:cidr_merged_sip_2"
                input_stream: "GIN_HostNicName2:0:host_nic_name2"
                input_stream: "GIN_Netdevs_2:0:netdevs_2"

                input_stream: "GIN_ParsedPackets_3:0:parsed_packets_3"
                input_stream: "GIN_CidrMergedSIP_3:0:cidr_merged_sip_3"
                input_stream: "GIN_HostNicName3:0:host_nic_name3"
                input_stream: "GIN_Netdevs_3:0:netdevs_3"

                input_stream: "GIN_ParsedPackets_4:0:parsed_packets_4"
                input_stream: "GIN_CidrMergedSIP_4:0:cidr_merged_sip_4"
                input_stream: "GIN_HostNicName4:0:host_nic_name4"
                input_stream: "GIN_Netdevs_4:0:netdevs_4"

                output_stream: "GOut_DMSRules:0:dms_rules"

                node {
                  name: "A"
                  task: "SndSIPBaseMergeTask"

                  input_side_data: "GSD_NumberRatioMin:0:number_ratio_min"
                  input_side_data: "GSD_NumberRatioMax:0:number_ratio_max"
                  input_side_data: "GSD_NumberStatType:0:number_stat_type"
                  input_side_data: "GSD_ProtocolRatioMin:0:protocol_ratio_min"
                  input_side_data: "GSD_ProtocolRatioMax:0:protocol_ratio_max"
                  input_side_data: "GSD_ProtocolStatType:0:protocol_stat_type"

                  input_stream: "GIN_ParsedPackets_1:0:parsed_packets_1"
                  input_stream: "GIN_CidrMergedSIP_1:0:cidr_merged_sip_1"
                  input_stream: "GIN_HostNicName1:0:host_nic_name1"

                  output_stream: "GOut_SIPCidrBasedPacketsMerge1:0:sip_cidr_based_packets_merge1"
                }

                node {
                  name: "B"
                  task: "SndSIPBaseMergeTask"

                  input_side_data: "GSD_NumberRatioMin:0:number_ratio_min"
                  input_side_data: "GSD_NumberRatioMax:0:number_ratio_max"
                  input_side_data: "GSD_NumberStatType:0:number_stat_type"
                  input_side_data: "GSD_ProtocolRatioMin:0:protocol_ratio_min"
                  input_side_data: "GSD_ProtocolRatioMax:0:protocol_ratio_max"
                  input_side_data: "GSD_ProtocolStatType:0:protocol_stat_type"

                  input_stream: "GIN_ParsedPackets_2:0:parsed_packets_2"
                  input_stream: "GIN_CidrMergedSIP_2:0:cidr_merged_sip_2"
                  input_stream: "GIN_HostNicName2:0:host_nic_name2"

                  output_stream: "GOut_SIPCidrBasedPacketsMerge2:0:sip_cidr_based_packets_merge2"
                }

                node {
                  name: "C"
                  task: "SndSIPBaseMergeTask"

                  input_side_data: "GSD_NumberRatioMin:0:number_ratio_min"
                  input_side_data: "GSD_NumberRatioMax:0:number_ratio_max"
                  input_side_data: "GSD_NumberStatType:0:number_stat_type"
                  input_side_data: "GSD_ProtocolRatioMin:0:protocol_ratio_min"
                  input_side_data: "GSD_ProtocolRatioMax:0:protocol_ratio_max"
                  input_side_data: "GSD_ProtocolStatType:0:protocol_stat_type"

                  input_stream: "GIN_ParsedPackets_3:0:parsed_packets_3"
                  input_stream: "GIN_CidrMergedSIP_3:0:cidr_merged_sip_3"
                  input_stream: "GIN_HostNicName3:0:host_nic_name3"

                  output_stream: "GOut_SIPCidrBasedPacketsMerge3:0:sip_cidr_based_packets_merge3"
                }

                node {
                  name: "D"
                  task: "SndSIPBaseMergeTask"

                  input_side_data: "GSD_NumberRatioMin:0:number_ratio_min"
                  input_side_data: "GSD_NumberRatioMax:0:number_ratio_max"
                  input_side_data: "GSD_NumberStatType:0:number_stat_type"
                  input_side_data: "GSD_ProtocolRatioMin:0:protocol_ratio_min"
                  input_side_data: "GSD_ProtocolRatioMax:0:protocol_ratio_max"
                  input_side_data: "GSD_ProtocolStatType:0:protocol_stat_type"

                  input_stream: "GIN_ParsedPackets_4:0:parsed_packets_4"
                  input_stream: "GIN_CidrMergedSIP_4:0:cidr_merged_sip_4"
                  input_stream: "GIN_HostNicName4:0:host_nic_name4"

                  output_stream: "GOut_SIPCidrBasedPacketsMerge4:0:sip_cidr_based_packets_merge4"
                }

                node {
                  name: "E"
                  task: "SndGenDMSRulesTask"

                  input_stream: "GOut_SIPCidrBasedPacketsMerge1:0:sip_cidr_based_packets_merge1"
                  input_stream: "GIN_Netdevs_1:0:netdevs_1"

                  input_stream: "GOut_SIPCidrBasedPacketsMerge2:0:sip_cidr_based_packets_merge2"
                  input_stream: "GIN_Netdevs_2:0:netdevs_2"

                  input_stream: "GOut_SIPCidrBasedPacketsMerge3:0:sip_cidr_based_packets_merge3"
                  input_stream: "GIN_Netdevs_3:0:netdevs_3"

                  input_stream: "GOut_SIPCidrBasedPacketsMerge4:0:sip_cidr_based_packets_merge4"
                  input_stream: "GIN_Netdevs_4:0:netdevs_4"

                  output_stream: "GOut_DMSRules:0:dms_rules"
                }
        )pb";

        auto gc = dni::ParseStringToGraphConfig(proto);
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

        auto ret =
            g->GetResult<std::unordered_map<std::string, std::vector<dni::snding::DMSRule>>>(out);
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
