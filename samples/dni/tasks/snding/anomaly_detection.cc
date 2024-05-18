#include <string>
#include <vector>

#include "dni/framework/framework.h"
#include "samples/tools/datumgen.h"
#include "spdlog/spdlog.h"

int main()
{
        spdlog::set_level(spdlog::level::trace);

        const std::string& proto =
            "samples/dni/tasks/snding/testdata/anomaly_detection.pbtxt";
        auto gc = dni::LoadTextprotoFile(proto);
        if (!gc)
        {
                spdlog::error("invalid pbtxt config: {}", proto);
                return -1;
        }

        dni::Graph* g = new dni::Graph(gc.value());
        DatumGen gen = DatumGen<int>(g);

        std::vector<InputMap> inputs = {
            {
                {"packets", dni::Datum(0.)},
                {"netdev", dni::Datum(0.)},
                {"resource", dni::Datum(0.)},
            },
            {
                {"packets", dni::Datum(0.)},
                {"netdev", dni::Datum(0.)},
                {"resource", dni::Datum(1.)},
            },
            {
                {"packets", dni::Datum(0.)},
                {"netdev", dni::Datum(1.)},
                {"resource", dni::Datum(0.)},
            },
            {
                {"packets", dni::Datum(0.)},
                {"netdev", dni::Datum(1.)},
                {"resource", dni::Datum(1.)},
            },
            {
                {"packets", dni::Datum(1.)},
                {"netdev", dni::Datum(0.)},
                {"resource", dni::Datum(0.)},
            },
            {
                {"packets", dni::Datum(1.)},
                {"netdev", dni::Datum(0.)},
                {"resource", dni::Datum(1.)},
            },
            {
                {"packets", dni::Datum(1.)},
                {"netdev", dni::Datum(1.)},
                {"resource", dni::Datum(0.)},
            },
            {
                {"packets", dni::Datum(1.)},
                {"netdev", dni::Datum(1.)},
                {"resource", dni::Datum(1.)},
            },
        };
        std::string out = "type";
        gen.Loop(inputs, out);

        g->Finish();

        spdlog::info("main over");

        return 0;
}
