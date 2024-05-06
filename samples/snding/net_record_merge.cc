#include <chrono>
#include <thread>

#include "dni/framework/framework.h"
#include "dni/tasks/snding/snding_defines.h"
#include "spdlog/spdlog.h"

/*
{
"16909056/24:84281096/32": {"target1#eno3", "A10-1#eno2"},
"16909056/24:84281097/32": {"target2#eno4", "A10-1#eno2"},
"2711790336/24:84281096/32": {"A10-2#eno2", "target1#eno3", "A10-1#eno2"},
"2711790336/24:84281097/32": {"target2#eno4", "A10-2#eno2", "A10-1#eno2"}
}

*/
void inject_after1(dni::Graph* g, int after, int n, int interval)
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

        // from A10-1
        std::unordered_map<std::string, dni::snding::SIPBaseMergeStats> map1;
        dni::snding::SIPBaseMergeStats s11;
        s11.srcIP = {0x01020300, 24};
        s11.dstIP.value.insert(0x05060708);
        s11.dstIP.value.insert(0x05060709);
        s11.hostNicSign = "A10-1#eno2";
        map1[std::to_string(0x01020300) + "/24"] = s11;

        dni::snding::SIPBaseMergeStats s13;
        s13.srcIP = {0xA1A2A300, 24};
        s13.dstIP.value.insert(0x05060708);
        s13.dstIP.value.insert(0x05060709);
        s13.hostNicSign = "A10-1#eno2";
        map1[std::to_string(0xA1A2A300) + "/24"] = s13;

        // from A10-2
        std::unordered_map<std::string, dni::snding::SIPBaseMergeStats> map4;
        dni::snding::SIPBaseMergeStats s41;
        s41.srcIP = {0xA1A2A300, 24};
        s41.dstIP.value.insert(0x05060708);
        s41.dstIP.value.insert(0x05060709);
        s41.hostNicSign = "A10-2#eno2";
        map4[std::to_string(0xA1A2A300) + "/24"] = s41;

        // from target1
        std::unordered_map<std::string, dni::snding::SIPBaseMergeStats> map2;
        dni::snding::SIPBaseMergeStats s21;
        s21.srcIP = {0x01020300, 24};
        s21.dstIP.value.insert(0x05060708);
        s21.hostNicSign = "target1#eno3";
        map2[std::to_string(0x01020300) + "/24"] = s21;

        dni::snding::SIPBaseMergeStats s22;
        s22.srcIP = {0xA1A2A300, 24};
        s22.dstIP.value.insert(0x05060708);
        s22.hostNicSign = "target1#eno3";
        map2[std::to_string(0xA1A2A300) + "/24"] = s22;

        // from target2
        std::unordered_map<std::string, dni::snding::SIPBaseMergeStats> map5;
        dni::snding::SIPBaseMergeStats s51;
        s51.srcIP = {0xA1A2A300, 24};
        s51.dstIP.value.insert(0x05060709);
        s51.hostNicSign = "target2#eno4";
        map5[std::to_string(0xA1A2A300) + "/24"] = s51;

        dni::snding::SIPBaseMergeStats s52;
        s52.srcIP = {0x01020300, 24};
        s52.dstIP.value.insert(0x05060709);
        s52.hostNicSign = "target2#eno4";
        map5[std::to_string(0x01020300) + "/24"] = s52;

        for (int i = 0; i < n; i++)
        {
                // SPDLOG_DEBUG(
                //     "send Datum({}) to graph g({:p})", dni::Datum(stat), fmt::ptr(g));
                g->AddDatumToInputStream("sip_merge0", dni::Datum(map1));
                g->AddDatumToInputStream("sip_merge1", dni::Datum(map2));
                g->AddDatumToInputStream("sip_merge2", dni::Datum(map4));
                g->AddDatumToInputStream("sip_merge3", dni::Datum(map5));

                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        }
}

void inject_after2(dni::Graph* g, int after, int n, int interval)
{
        std::this_thread::sleep_for(std::chrono::milliseconds(after));

        /*

                                                                          (5.6.7.9/32)
                                                          |-------------> target2:eno4
                     SIP1                                 |
        attacker0:eno1 ~ attacker3:eno1------------> A10-1:eno2 --------> target1:eno3
                1.2.3.0/30                          10.10.10.10            (5.6.7.8/32)

                                                         /|\
                                                          |
                                                          |
                                                          |
                      SIP2
        attacker256:eno1 ~ attacker511:eno1--------> A10-2:eno2
                A1.A2.A3.0/24                       20.20.20.20


        */

        // from A10-1
        std::unordered_map<std::string, dni::snding::SIPBaseMergeStats> map1;
        dni::snding::SIPBaseMergeStats s11;
        s11.srcIP = {0x01020300, 30};
        s11.dstIP.value.insert(0x05060708);
        s11.dstIP.value.insert(0x05060709);
        s11.hostNicSign = "A10-1#eno2";
        map1[std::to_string(0x01020300) + "/30"] = s11;

        dni::snding::SIPBaseMergeStats s13;
        s13.srcIP = {0xA1A2A300, 24};
        s13.dstIP.value.insert(0x05060708);
        s13.dstIP.value.insert(0x05060709);
        s13.hostNicSign = "A10-1#eno2";
        map1[std::to_string(0xA1A2A300) + "/24"] = s13;

        // from A10-2
        std::unordered_map<std::string, dni::snding::SIPBaseMergeStats> map4;
        dni::snding::SIPBaseMergeStats s41;
        s41.srcIP = {0xA1A2A300, 24};
        s41.dstIP.value.insert(0x05060708);
        s41.dstIP.value.insert(0x05060709);
        s41.hostNicSign = "A10-2#eno2";
        map4[std::to_string(0xA1A2A300) + "/24"] = s41;

        // from target1
        std::unordered_map<std::string, dni::snding::SIPBaseMergeStats> map2;
        dni::snding::SIPBaseMergeStats s21;
        s21.srcIP = {0x01020300, 30};
        s21.dstIP.value.insert(0x05060708);
        s21.hostNicSign = "target1#eno3";
        map2[std::to_string(0x01020300) + "/30"] = s21;

        dni::snding::SIPBaseMergeStats s22;
        s22.srcIP = {0xA1A2A300, 24};
        s22.dstIP.value.insert(0x05060708);
        s22.hostNicSign = "target1#eno3";
        map2[std::to_string(0xA1A2A300) + "/24"] = s22;

        // from target2
        std::unordered_map<std::string, dni::snding::SIPBaseMergeStats> map5;
        dni::snding::SIPBaseMergeStats s51;
        s51.srcIP = {0xA1A2A300, 24};
        s51.dstIP.value.insert(0x05060709);
        s51.hostNicSign = "target2#eno4";
        map5[std::to_string(0xA1A2A300) + "/24"] = s51;

        dni::snding::SIPBaseMergeStats s52;
        s52.srcIP = {0x01020300, 30};
        s52.dstIP.value.insert(0x05060709);
        s52.hostNicSign = "target2#eno4";
        map5[std::to_string(0x01020300) + "/30"] = s52;

        for (int i = 0; i < n; i++)
        {
                // SPDLOG_DEBUG(
                //     "send Datum({}) to graph g({:p})", dni::Datum(stat), fmt::ptr(g));
                g->AddDatumToInputStream("sip_merge0", dni::Datum(map1));
                g->AddDatumToInputStream("sip_merge1", dni::Datum(map2));
                g->AddDatumToInputStream("sip_merge2", dni::Datum(map4));
                g->AddDatumToInputStream("sip_merge3", dni::Datum(map5));

                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        }
}

int main()
{
        spdlog::set_level(spdlog::level::trace);

        const std::string& proto = R"pb(
                type: "SndNetRecordMerge"

                input_stream: "GIn_SIPMerge0:0:sip_merge0"
                input_stream: "GIn_SIPMerge1:0:sip_merge1"
                input_stream: "GIn_SIPMerge2:0:sip_merge2"
                input_stream: "GIn_SIPMerge3:0:sip_merge3"

                output_stream: "GOut:0:attack_link"

                node {
                  name: "A"
                  task: "SndNetRecordMergeTask"
                  input_stream: "GIn_SIPMerge0:0:sip_merge0"
                  input_stream: "GIn_SIPMerge1:0:sip_merge1"
                  input_stream: "GIn_SIPMerge2:0:sip_merge2"
                  input_stream: "GIn_SIPMerge3:0:sip_merge3"

                  output_stream: "GOut:0:attack_link"
                }
        )pb";

        // const std::string& proto = R"pb(
        //         type: "SndNetRecordMerge"

        //         input_stream: "GIn_SIPMerge:0:sip_merge0"
        //         input_stream: "GIn_SIPMerge:1:sip_merge1"
        //         input_stream: "GIn_SIPMerge:2:sip_merge2"
        //         input_stream: "GIn_SIPMerge:3:sip_merge3"

        //         output_stream: "GOut:0:attack_link"

        //         node {
        //           name: "A"
        //           task: "SndNetRecordMergeTask"
        //           input_stream: "GIn_SIPMerge:0:sip_merge0"
        //           input_stream: "GIn_SIPMerge:1:sip_merge1"
        //           input_stream: "GIn_SIPMerge:2:sip_merge2"
        //           input_stream: "GIn_SIPMerge:3:sip_merge3"

        //           output_stream: "GOut:0:attack_link"
        //         }
        // )pb";

        auto gc = dni::ParseStringToGraphConfig(proto);
        if (!gc)
        {
                spdlog::error("invalid pbtxt config: {}", proto);
                return -1;
        }
        spdlog::debug(
            "GraphConfig: {}, node size: {}", gc.value().type(), gc.value().node_size());

        dni::Graph* g = new dni::Graph(gc.value());

        std::string out = "attack_link";

        spdlog::debug("Create ObserveOutputStream: {}", out);
        g->ObserveOutputStream(out);

        g->PrepareForRun();

        inject_after2(g, 0, 1, 0);

        g->RunOnce();

        g->Wait();

        auto ret = g->GetResult<
            std::unordered_map<std::string, std::unordered_set<std::string>>>(out);
        spdlog::info("Gout {} result size is: {}", out, ret.size());
        spdlog::info("Gout {} result is: {}", out, ret);

        g->Finish();

        spdlog::info("main over");

        return 0;
}
