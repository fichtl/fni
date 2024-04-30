#include <chrono>
#include <thread>

#include "dni/framework/framework.h"
#include "spdlog/spdlog.h"
#include "snding_defines.h"

void inject_after(
    dni::Graph* g, int after, int n, int interval)
{
        std::this_thread::sleep_for(std::chrono::milliseconds(after));

        /*
        attacker0:eno1 ~ attacker3:eno1--------> A10-1:eno2 --------> target1:eno3
                1.2.3.0/30                      10.10.10.10             5.6.7.8/32


        attacker256:eno1 ~ attacker511:eno1--------> A10-2:eno2--------> A10-1:eno2 --------> target2:eno4
                A1.A2.A3.0/24                       20.20.20.20         10.10.10.10          A5.A6.A7.A8/32

        */

        std::vector<std::unordered_map<std::string, dni::SIPBaseMergeStats>> net_record;

        std::unordered_map<std::string, dni::SIPBaseMergeStats> map1; // from A10-1
        dni::SIPBaseMergeStats s1;
        s1.srcIP = {0x01020300, 30};
        s1.dstIP = {0x05060708, 32};
        s1.hostNicSign = "A10-1#eno2";
        map1[std::to_string(0x01020300) + "/30"] = s1;


        std::unordered_map<std::string, dni::SIPBaseMergeStats> map2; // from target1
        dni::SIPBaseMergeStats s2;
        s2.srcIP = {0x01020300, 30};
        s2.dstIP = {0x05060708, 32};
        s2.hostNicSign = "target1#eno3";
        map2[std::to_string(0x01020300) + "/30"] = s2;


        //
        std::unordered_map<std::string, dni::SIPBaseMergeStats> map3; // from A10-1
        dni::SIPBaseMergeStats s3;
        s3.srcIP = {0xA1A2A300, 24};
        s3.dstIP = {0xA5A6A7A8, 32};
        s3.hostNicSign = "A10-1#eno2";
        map3[std::to_string(0xA1A2A300) + "/24"] = s3;


        std::unordered_map<std::string, dni::SIPBaseMergeStats> map4; // from A10-2
        dni::SIPBaseMergeStats s4;
        s4.srcIP = {0xA1A2A300, 24};
        s4.dstIP = {0xA5A6A7A8, 32};
        s4.hostNicSign = "A10-2#eno2";
        map4[std::to_string(0xA1A2A300) + "/24"] = s4;


        std::unordered_map<std::string, dni::SIPBaseMergeStats> map5; // from target2
        dni::SIPBaseMergeStats s5;
        s5.srcIP = {0xA1A2A300, 24};
        s5.dstIP = {0xA5A6A7A8, 32};
        s5.hostNicSign = "target2#eno4";
        map5[std::to_string(0xA1A2A300) + "/24"] = s5;


        net_record.emplace_back(std::move(map5));
        net_record.emplace_back(std::move(map1));
        net_record.emplace_back(std::move(map3));
        net_record.emplace_back(std::move(map2));
        net_record.emplace_back(std::move(map4));

        for (int i = 0; i < n; i++)
        {
                SPDLOG_DEBUG("send Datum({}) to graph g({:p})", dni::Datum(stat), fmt::ptr(g));
                g->AddDatumToInputStream("net_record", dni::Datum(net_record));

                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        }
}

int main()
{
        spdlog::set_level(spdlog::level::trace);

        const std::string& proto = R"pb(
                type: "NetRecordMerge"

                input_stream: "GIn:0:net_record"
                output_stream: "GOut:0:attack_link"

                node {
                  name: "A"
                  task: "NetRecordMergeTask"
                  input_stream: "GIn:0:net_record"
                  output_stream: "GOut:0:attack_link"
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

        std::string out = "attack_link";

        spdlog::debug("Create ObserveOutputStream: {}", out);
        g->ObserveOutputStream(out);

        g->PrepareForRun();

        inject_after(g, 0, 1, 0);

        g->RunOnce();

        g->Wait();

        auto ret = g->GetResult<std::unordered_map<std::string, std::unordered_set<std::string>>>(out);
        spdlog::info("Gout {} result is: {}", out, ret);

        g->Finish();

        spdlog::info("main over");

        return 0;
}