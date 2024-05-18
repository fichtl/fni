#include <string>
#include <vector>

#include "dni/framework/framework.h"
#include "samples/tools/datumgen.h"
#include "spdlog/spdlog.h"

int main()
{
        spdlog::set_level(spdlog::level::trace);

        const std::string& proto = "samples/ddos/graph1/testdata/graph1_a.pbtxt";
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
                {"countTotal", dni::Datum(13000000.)},
            },
        };
        std::string out = "packet_countTotal";
        gen.Loop(inputs, out);

        g->Finish();

        spdlog::info("main over");

        return 0;
}
