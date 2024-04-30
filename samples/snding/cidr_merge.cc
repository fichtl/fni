#include <chrono>
#include <thread>

#include "dni/framework/framework.h"
#include "snding_defines.h"
#include "spdlog/spdlog.h"

void inject_after(dni::Graph* g, int after, int n, int interval)
{
        std::this_thread::sleep_for(std::chrono::milliseconds(after));

        std::unordered_map<uint32_t, int> ip_map;
        uint32_t start = 0x01020000;
        for (size_t i = 0; i < 256; i++)
        {
                ip_map[start + i] = 10;
        }

        //
        start = 0x01020100;
        for (size_t i = 0; i < 256; i++)
        {
                ip_map[start + i] = 10;
        }

        //
        start = 0x01020200;
        for (size_t i = 0; i < 256; i++)
        {
                ip_map[start + i] = 10;
        }

        //
        start = 0x01020300;
        for (size_t i = 0; i < 256; i++)
        {
                ip_map[start + i] = 10;
        }

        //
        start = 0x01020400;
        for (size_t i = 0; i < 256; i++)
        {
                ip_map[start + i] = 10;
        }

        //
        start = 0x01020500;
        for (size_t i = 0; i < 256; i++)
        {
                ip_map[start + i] = 10;
        }

        //
        start = 0x01020600;
        for (size_t i = 0; i < 256; i++)
        {
                ip_map[start + i] = 10;
        }

        //
        start = 0x01020700;
        for (size_t i = 0; i < 256; i++)
        {
                ip_map[start + i] = 10;
        }

        for (int i = 0; i < n; i++)
        {
                SPDLOG_DEBUG(
                    "send Datum({}) to graph g({:p})", dni::Datum(stat), fmt::ptr(g));
                g->AddDatumToInputStream("ip_map", dni::Datum(ip_map));

                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        }
}

void inject_after1(dni::Graph* g, int after, int n, int interval)
{
        std::this_thread::sleep_for(std::chrono::milliseconds(after));

        std::unordered_map<uint32_t, int> ip_map;
        uint32_t start = 0x01020000;
        for (size_t i = 0; i < 256; i++)
        {
                ip_map[start + i] = 10;
        }

        //
        start = 0x01020100;
        for (size_t i = 0; i < 256; i++)
        {
                ip_map[start + i] = 10;
        }

        for (int i = 0; i < n; i++)
        {
                SPDLOG_DEBUG(
                    "send Datum({}) to graph g({:p})", dni::Datum(stat), fmt::ptr(g));
                g->AddDatumToInputStream("ip_map", dni::Datum(ip_map));

                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        }
}

void inject_after2(dni::Graph* g, int after, int n, int interval)
{
        std::this_thread::sleep_for(std::chrono::milliseconds(after));

        std::unordered_map<uint32_t, int> ip_map;
        uint32_t start = 0x01020000;
        for (size_t i = 0; i < 256; i++)
        {
                ip_map[start + i] = 10;
        }

        //
        start = 0x01020100;
        for (size_t i = 0; i < 256; i++)
        {
                ip_map[start + i] = 10;
        }

        //
        start = 0x01020200;
        for (size_t i = 0; i < 256; i++)
        {
                ip_map[start + i] = 10;
        }

        for (int i = 0; i < n; i++)
        {
                SPDLOG_DEBUG(
                    "send Datum({}) to graph g({:p})", dni::Datum(stat), fmt::ptr(g));
                g->AddDatumToInputStream("ip_map", dni::Datum(ip_map));

                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        }
}

int main()
{
        spdlog::set_level(spdlog::level::trace);

        const std::string& proto = R"pb(
                type: "CIDRMerge"

                input_stream: "GIn:0:ip_map"
                output_stream: "GOut:0:cidr_merged_sip"

                node {
                  name: "A"
                  task: "CIDRMergeTask"
                  input_stream: "GIn:0:ip_map"
                  output_stream: "GOut:0:cidr_merged_sip"
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

        std::string out = "cidr_merged_sip";

        spdlog::debug("Create ObserveOutputStream: {}", out);
        g->ObserveOutputStream(out);

        g->PrepareForRun();

        inject_after(g, 0, 1, 0);

        g->RunOnce();

        g->Wait();

        auto ret = g->GetResult<std::vector<dni::IP>>(out);
        for (auto&& ip : ret)
        {
                spdlog::info("Gout {} result is: {:x}/{}", out, ip.ip, ip.len);
        }

        g->Finish();

        spdlog::info("main over");

        return 0;
}
