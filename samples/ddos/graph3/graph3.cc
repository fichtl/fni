#include <chrono>
#include <map>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "dni/framework/framework.h"
#include "dni/tasks/snding/snding_defines.h"
#include "fmt/format.h"
#include "spdlog/spdlog.h"

#define PCAP_DUMP_FILE_PATH "samples/ddos/graph3/testdata/test{}.pcap"

void inject_after(
    dni::Graph* g, std::map<std::string, dni::Datum> input, int after, int n,
    int interval)
{
        std::this_thread::sleep_for(std::chrono::milliseconds(after));

        for (int i = 0; i < n; i++)
        {
                for (auto& kv : input)
                {
                        g->AddDatumToInputStream(kv.first, kv.second);
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        }
}

int main()
{
        spdlog::set_level(spdlog::level::trace);

        const std::string& proto = "samples/ddos/graph3/testdata/graph3.pbtxt";
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

        std::vector<uint32_t> all_known_ips = {};

        std::vector<double_t> netdevs;
        netdevs.push_back(4.00 * 1e9);
        netdevs.push_back(0.6);
        netdevs.push_back(16.00 * 1e6);
        netdevs.push_back(1.00);

        std::vector<std::map<std::string, dni::Datum>> inputs;
        for (int i = 3; i < 9; i++)
        {
                std::string fpath = fmt::format(PCAP_DUMP_FILE_PATH, i);
                inputs.push_back(
                    {{"pcapPath", dni::Datum(fpath)},
                     {"all_known_ips", dni::Datum(all_known_ips)},
                     {"host_nic_name", dni::Datum(std::string("target1#eno3"))},
                     {"netdevs_1", dni::Datum(netdevs)}});
        }

        int i = 0;
        for (auto& v : inputs)
        {
                auto ts = std::chrono::system_clock::now();
                spdlog::info("TEST CASE {} STARTS ------------------------------\n", i);

                inject_after(g, v, 0, 1, 0);

                g->RunOnce();

                g->Wait();

                auto ret = g->GetResult<
                    std::unordered_map<std::string, std::vector<dni::snding::DMSRule>>>(
                    out);
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

                spdlog::info("TEST CASE {} ENDS ------------------------------\n", i);
                spdlog::info(
                    "TEST CASE {} TIME ELAPSE: {}ms", i++,
                    std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now() - ts)
                        .count());
        }

        g->Finish();

        spdlog::info("main over");

        return 0;
}
