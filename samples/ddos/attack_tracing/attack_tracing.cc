#include "dni/framework/framework.h"
#include "dni/tasks/snding/snding_defines.h"
#include "samples/tools/datumgen.h"
#include "spdlog/spdlog.h"

int main()
{
        spdlog::set_level(spdlog::level::trace);

        const std::string& proto =
            "samples/ddos/attack_tracing/testdata/attack_tracing.pbtxt";
        auto gc = dni::LoadTextprotoFile(proto);
        if (!gc)
        {
                spdlog::error("invalid pbtxt config: {}", proto);
                return -1;
        }

        dni::Graph* g = new dni::Graph(gc.value());
        DatumGen gen = DatumGen<dni::snding::AttackerIPMergeResult>(g);

        std::string pcapPath = "samples/ddos/attack_tracing/testdata/attack_tracing.pcap";
        std::vector<InputMap> inputs = {
            {
                {"pcapPath", dni::Datum(pcapPath)},
                {"all_known_ips", dni::Datum(std::vector<uint32_t>{})},
            },
        };
        std::string out = "attackerip_merge_result";
        gen.Loop(inputs, out);

        g->Finish();

        spdlog::info("main over");

        return 0;
}
