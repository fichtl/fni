#include <string>
#include <unordered_map>
#include <vector>

#include "dni/framework/framework.h"
#include "samples/tools/datumgen.h"
#include "spdlog/spdlog.h"

int main()
{
        spdlog::set_level(spdlog::level::trace);

        const std::string& proto = "samples/dni/tasks/snding/testdata/pcap_parse.pbtxt";
        auto gc = dni::LoadTextprotoFile(proto);
        if (!gc)
        {
                spdlog::error("invalid pbtxt config: {}", proto);
                return -1;
        }

        dni::Graph* g = new dni::Graph(gc.value());
        DatumGen gen =
            DatumGen<std::vector<std::unordered_map<std::string, uint32_t>>>(g);

        std::string pcap_fpath =
            "samples/dni/tasks/snding/testdata/pcap_parse_vlan_id.pcap";
        std::vector<InputMap> inputs = {
            {
                {"pcap", dni::Datum(pcap_fpath)},
            },
        };
        std::string out = "out";
        gen.Loop(inputs, out);

        g->Finish();

        spdlog::info("main over");

        return 0;
}
