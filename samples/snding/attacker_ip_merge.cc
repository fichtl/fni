#include <chrono>
#include <thread>

#include "dni/framework/framework.h"
#include "spdlog/spdlog.h"
#include "dni/tasks/snding/snding_defines.h"

#define PCAP_DUMP_FILE_PATH "samples/snding/testdata/test1.pcap"


void inject_after(
    dni::Graph* g, int after, int n, int interval)
{
        std::this_thread::sleep_for(std::chrono::milliseconds(after));

        std::vector<uint32_t> all_known_ips = {};

        for (int i = 0; i < n; i++)
        {
                SPDLOG_DEBUG(
                    "send Datum({}) to graph g({:p})",
                    dni::Datum(std::string(PCAP_DUMP_FILE_PATH)), fmt::ptr(g));

                // abnormal_nic_pcap_parser
                g->AddDatumToInputStream(
                    "pcapPath", dni::Datum(std::string(PCAP_DUMP_FILE_PATH)));

                 g->AddDatumToInputStream("all_known_ips", dni::Datum(all_known_ips));

                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        }

}


int main()
{
        spdlog::set_level(spdlog::level::trace);

        const std::string& proto = R"pb(

            type: "Attack Tracing and Defense"

            # abnormal nic pcap file path
            input_stream: "GIn_pcapPath:0:pcapPath"

            # attack_ip_merge
            input_stream: "GIn_All_Known_IPs:0:all_known_ips"

            # graph3 output
            output_stream: "NOut_Attackerip_Merge_Result:0:attackerip_merge_result"

            node {
                name: "abnormal_nic_pcap_parser"
                task: "PcapParseTask"
                input_stream: "GIn_pcapPath:0:pcapPath"
                output_stream: "GOut_parsed_packets:0:parsed_packets"
            }


            node {
                name: "abnormal_pcap_feature_counter"
                task: "FeatureCounterTask"
                input_stream: "GOut_parsed_packets:0:parsed_packets"
                output_stream: "NOut_SIP_COUNT:0:sip_count"
                output_stream: "NOut_SPort_COUNT:0:sport_count"
                output_stream: "NOut_DPort_COUNT:0:dport_count"
                output_stream: "NOut_Protocol_COUNT:0:protocol_count"
                output_stream: "NOut_Length_COUNT:0:length_count"
                options {
                    [type.asnapis.io/dni.FeatureCounterTaskOptions] {
                        feature: "SIP"
                        feature: "SPort"
                        feature: "DPort"
                        feature: "Protocol"
                        feature: "Length"
                    }
                }
            }

            node {
                name: "attack_ip_merge"
                task: "SndAttackerIPMergeTask"
                input_stream: "NOut_SIP_COUNT:0:sip_count"
                input_stream: "GIn_All_Known_IPs:0:all_known_ips"
                output_stream: "NOut_Attackerip_Merge_Result:0:attackerip_merge_result"
                options {
                    [type.asnapis.io/dni.SndAttackerIPMergeTaskOptions] {
                        ipFw4CountRatio: 0.1
                        ipFw3CountRatio: 0.2
                        ipSegCoverThreshold: 100
                        ipFw2CountRatio: 0.4
                        ipRandCountThreshold: 2
                        ipRandCountRatio: 0.5
                    }
                }
            }

        )pb";

        // spdlog::debug("pbtxt config: {}", proto);

        auto gc = dni::ParseStringToGraphConfig(proto);
        if (!gc)
        {
                spdlog::error("invalid pbtxt config: {}", proto);
                return -1;
        }

        spdlog::debug(
            "GraphConfig: {}, node size: {}", gc.value().type(), gc.value().node_size());

        dni::Graph* g = new dni::Graph(gc.value());

        std::string out = "attackerip_merge_result";
        spdlog::debug("Create ObserveOutputStream: {}", out);
        g->ObserveOutputStream(out);

        // std::vector<std::string> out_stream_names = {
        //     "sip_count", "sport_count", "dport_count", "protocol_count", "length_count"};
        // for (const auto& name : out_stream_names)
        // {
        //         spdlog::debug("Create ObserveOutputStream: {}", name);
        //         g->ObserveOutputStream(name);
        // }
        spdlog::info("main 1");

        g->PrepareForRun();
        spdlog::info("main 2");
        spdlog::info("main 3");
        inject_after(g, 0, 1, 0);
        spdlog::info("main 4");

        g->RunOnce();
        g->Wait();

        auto ret = g->GetResult<dni::snding::AttackerIPMergeResult>(out);
        // auto ret = std::move(
        // g->GetResult<std::vector<std::unordered_map<std::string, uint32_t>>>(out));

        spdlog::info("Gout {} result is: {}", out, ret);

        // for (const auto& name : out_stream_names)
        // {
        //         auto ret = g->GetResult<std::unordered_map<uint32_t, int>>(name);
        //         spdlog::info("Gout {} result is: {}", name, ret);
        // }

        g->Finish();

        spdlog::info("main over");

        return 0;
}




