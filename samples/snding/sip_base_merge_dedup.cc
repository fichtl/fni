
#include <chrono>
#include <thread>

#include "dni/framework/framework.h"
#include "dni/tasks/datagram/cidr.h"
#include "dni/tasks/snding/snding_defines.h"
#include "spdlog/spdlog.h"

void inject_after1(dni::Graph* g, int after, int n, int interval)
{
        std::this_thread::sleep_for(std::chrono::milliseconds(after));

        srand(time(NULL));

        std::vector<std::unordered_map<std::string, uint32_t>> parsed_packets;
        for (size_t i = 0; i < 10000; i++)
        {
                std::unordered_map<std::string, uint32_t> p1;

                p1["SIP"] = 0x01020300 + rand() % 256;   // 0~255
                p1["DIP"] = 0x05060708;
                p1["SPort"] = rand() % 65536;   // 1~65535
                p1["DPort"] = 5678;
                p1["Protocol"] = 17;
                p1["Length"] = rand() % 1300 + 100;   // 100~1400
                parsed_packets.emplace_back(p1);
        }

        dni::snding::AttackerIPMergeResult attackMerge;
        std::unordered_map<uint32_t, int> ip_map;
        for (auto&& packet : parsed_packets)
        {
                ip_map[packet["SIP"]] = 0;   // do not care value of map
        }
        attackMerge.attackerIPs = std::move(dni::cidr_merge(ip_map));

        for (int i = 0; i < n; i++)
        {
                // SPDLOG_DEBUG("send Datum({}) to graph g({:p})", d, fmt::ptr(g));
                g->AddDatumToInputStream("parsed_packets", dni::Datum(parsed_packets));
                g->AddDatumToInputStream("cidr_merged_sip", dni::Datum(attackMerge));
                g->AddDatumToInputStream(
                    "host_nic_name", dni::Datum(std::string("target1#eno3")));

                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        }
}

void inject_after2(dni::Graph* g, int after, int n, int interval)
{
        std::this_thread::sleep_for(std::chrono::milliseconds(after));

        srand(time(NULL));

        std::vector<std::unordered_map<std::string, uint32_t>> parsed_packets;
        for (size_t i = 0; i < 3456; i++)
        {
                std::unordered_map<std::string, uint32_t> p1;

                p1["SIP"] = 0x01020300 + rand() % 256;   // 0~255
                p1["DIP"] = 0x05060708;
                p1["SPort"] = rand() % 65536;   // 1~65535
                p1["DPort"] = 5678;
                p1["Protocol"] = 17;
                p1["Length"] = rand() % 1300 + 100;   // 100~1400
                parsed_packets.emplace_back(p1);
        }

        for (size_t i = 0; i < (10000 - 3456); i++)
        {
                std::unordered_map<std::string, uint32_t> p1;

                p1["SIP"] = 0xa1a2a300 + rand() % 256;   // 0~255
                p1["DIP"] = 0x05060708;
                p1["SPort"] = rand() % 65536;   // 1~65535
                p1["DPort"] = (i % 2 == 0 ? 22222 : 33333);
                p1["Protocol"] = (i % 2 == 0 ? 17 : 6);
                p1["Length"] = rand() % 1300 + 100;   // 100~1400
                parsed_packets.emplace_back(p1);
        }

        dni::snding::AttackerIPMergeResult attackMerge;
        std::unordered_map<uint32_t, int> ip_map;
        for (auto&& packet : parsed_packets)
        {
                ip_map[packet["SIP"]] = 0;   // do not care value of map
        }
        attackMerge.attackerIPs = std::move(dni::cidr_merge(ip_map));

        for (int i = 0; i < n; i++)
        {
                // SPDLOG_DEBUG("send Datum({}) to graph g({:p})", d, fmt::ptr(g));
                g->AddDatumToInputStream("parsed_packets", dni::Datum(parsed_packets));
                g->AddDatumToInputStream("cidr_merged_sip", dni::Datum(attackMerge));
                g->AddDatumToInputStream(
                    "host_nic_name", dni::Datum(std::string("target1#eno3")));

                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        }
}

int main()
{
        spdlog::set_level(spdlog::level::trace);

        const std::string& proto = R"pb(
                type: "SndSIPBaseMerge"

                input_stream: "GIN_ParsedPackets:0:parsed_packets"
                input_stream: "GIN_CidrMergedSIP:0:cidr_merged_sip"
                input_stream: "GIN_HostNicName:0:host_nic_name"

                output_stream: "GOut_SIPCidrBasedPacketsMerge:0:sip_cidr_based_packets_merge"
                output_stream: "GOut_SIPDMSRulesPrepare:0:sip_dms_rules_prepare"

                node {
                  name: "A"
                  task: "SndSIPBaseMergeDeDupTask"

                  input_stream: "GIN_ParsedPackets:0:parsed_packets"
                  input_stream: "GIN_CidrMergedSIP:0:cidr_merged_sip"
                  input_stream: "GIN_HostNicName:0:host_nic_name"

                  output_stream: "GOut_SIPCidrBasedPacketsMerge:0:sip_cidr_based_packets_merge"
                  output_stream: "GOut_SIPDMSRulesPrepare:0:sip_dms_rules_prepare"

                  options {
                    [type.asnapis.io/dni.SndSIPBaseMergeDeDupTaskOptions] {
                      num_stat {
                        ratioMin: 0.1
                        ratioMax: 0.6
                        label: "centralize"
                        label: "regular"
                        label: "random"
                        label: "void"
                      }
                      proto_stat {
                        ratioMin: 0.1
                        ratioMax: 0.6
                        label: "flood"
                        label: "rapid"
                        label: "void"
                      }
                    }
                  }
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

        std::string out = "sip_cidr_based_packets_merge";

        spdlog::debug("Create ObserveOutputStream: {}", out);
        g->ObserveOutputStream(out);

        std::string out2 = "sip_dms_rules_prepare";

        spdlog::debug("Create ObserveOutputStream: {}", out2);
        g->ObserveOutputStream(out2);

        g->PrepareForRun();

        inject_after1(g, 0, 1, 0);

        g->RunOnce();

        g->Wait();

        auto ret =
            g->GetResult<std::unordered_map<std::string, dni::snding::SIPBaseMergeStats>>(
                out);
        spdlog::info("Gout {} result size is: {}", out, ret.size());
        for (auto&& stat : ret)
        {
                spdlog::info(
                    "Gout {} result is:\n cidr-ip:{}\n  {}\n\n", out, stat.first,
                    stat.second);
        }

        auto ret2 =
            g->GetResult<std::unordered_map<std::string, std::unordered_map<std::string, std::string>>>(
                out2);
        spdlog::info("Gout {} result size is: {}", out2, ret2.size());
        for (auto&& rule_stat : ret2)
        {
                spdlog::info(
                    "Gout {} result is:\n cidr-ip:{}\n  {}\n\n", out2, rule_stat.first,
                    rule_stat.second);
        }

        g->Finish();

        spdlog::info("main over");

        return 0;
}

