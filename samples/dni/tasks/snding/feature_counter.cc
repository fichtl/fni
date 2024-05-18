#include <chrono>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "dni/framework/framework.h"
#include "samples/tools/datumgen.h"
#include "spdlog/spdlog.h"

void inject_after(dni::Graph* g, int after, int n, int interval)
{
        std::this_thread::sleep_for(std::chrono::milliseconds(after));

        std::vector<std::unordered_map<std::string, uint32_t>> packets;
        std::unordered_map<std::string, uint32_t> p1;
        p1["SIP"] = 0x01020304;
        p1["DIP"] = 0x05060708;
        p1["SPort"] = 1234;
        p1["DPort"] = 5678;
        p1["Protocol"] = 17;
        p1["Length"] = 1300;
        packets.emplace_back(p1);
        packets.emplace_back(p1);
        packets.emplace_back(std::move(p1));

        std::unordered_map<std::string, uint32_t> p2;
        p2["SIP"] = 0x0a0b0c0d;
        p2["DIP"] = 0x05060708;
        p2["SPort"] = 1334;
        p2["DPort"] = 9527;
        p2["Protocol"] = 17;
        p2["Length"] = 1300;
        packets.emplace_back(p2);
        packets.emplace_back(std::move(p2));

        std::vector<std::string> feats = {"SIP", "SPort", "DPort", "Protocol", "Length"};

        for (int i = 0; i < n; i++)
        {
                g->AddDatumToInputStream("pcap_packets", dni::Datum(packets));

                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        }
}

int main()
{
        spdlog::set_level(spdlog::level::trace);

        const std::string& proto =
            "samples/dni/tasks/snding/testdata/feature_counter.pbtxt";
        auto gc = dni::LoadTextprotoFile(proto);
        if (!gc)
        {
                spdlog::error("invalid pbtxt config: {}", proto);
                return -1;
        }

        dni::Graph* g = new dni::Graph(gc.value());

        std::vector<std::string> out_stream_names = {
            "sip_count", "sport_count", "dport_count", "protocol_count", "length_count"};
        for (const auto& name : out_stream_names)
        {
                spdlog::debug("Create ObserveOutputStream: {}", name);
                g->ObserveOutputStream(name);
        }

        g->PrepareForRun();

        inject_after(g, 0, 1, 0);

        g->RunOnce();

        g->Wait();

        for (const auto& name : out_stream_names)
        {
                auto ret = g->GetResult<std::unordered_map<uint32_t, int>>(name);
                spdlog::info("Gout {} result is: {}", name, ret);
        }

        g->Finish();

        spdlog::info("main over");

        return 0;
}
