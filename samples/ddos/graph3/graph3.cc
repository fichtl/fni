#include <chrono>
#include <map>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "dni/framework/framework.h"
#include "dni/tasks/snding/snding_defines.h"
#include "fmt/format.h"
#include "samples/tools/datumgen.h"
#include "spdlog/spdlog.h"

#define PCAP_DUMP_FILE_PATH "samples/ddos/graph3/testdata/test{}.pcap"

using namespace std::string_literals;

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

        dni::Graph* g = new dni::Graph(gc.value());
        DatumGen gen = DatumGen<dni::snding::NICDMSRulesMap>(g);

        std::vector<uint32_t> all_known_ips = {};
        std::vector<double_t> netdevs;
        netdevs.push_back(4.00 * 1e9);
        netdevs.push_back(0.6);
        netdevs.push_back(16.00 * 1e6);
        netdevs.push_back(1.00);
        std::vector<InputMap> inputs;
        for (int i = 1; i < 9; i++)
        {
                std::string fpath = fmt::format(PCAP_DUMP_FILE_PATH, i);
                inputs.push_back({
                    {"pcapPath", dni::Datum(fpath)},
                    {"all_known_ips", dni::Datum(all_known_ips)},
                    {"host_nic_name", dni::Datum("target1#eno3"s)},
                    {"netdevs_1", dni::Datum(netdevs)},
                });
        }
        std::string out = "dms_rules";
        gen.Loop(inputs, out);

        g->Finish();

        spdlog::info("main over");

        return 0;
}
