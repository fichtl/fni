#include <string>
#include <vector>

#include "dni/framework/framework.h"
#include "samples/tools/datumgen.h"
#include "spdlog/spdlog.h"

using namespace std::string_literals;

int main()
{
        spdlog::set_level(spdlog::level::trace);

        const std::string& proto = "samples/ddos/graph2/testdata/graph2.pbtxt";
        auto gc = dni::LoadTextprotoFile(proto);
        if (!gc)
        {
                spdlog::error("invalid pbtxt config: {}", proto);
                return -1;
        }

        dni::Graph* g = new dni::Graph(gc.value());
        DatumGen gen = DatumGen<double_t>(g);

        std::vector<InputMap> inputs = {
            {
                {"pcapPath", dni::Datum("samples/ddos/graph2/testdata/test1.pcap"s)},
            },
        };
        std::string out = "attack_res";
        gen.Loop(inputs, out);

        g->Finish();

        spdlog::info("main over");

        return 0;
}
