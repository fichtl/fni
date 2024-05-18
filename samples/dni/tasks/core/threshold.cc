#include "dni/framework/framework.h"
#include "samples/tools/datumgen.h"
#include "spdlog/spdlog.h"

int main()
{
        spdlog::set_level(spdlog::level::trace);

        const std::string& proto = "samples/dni/tasks/core/testdata/threshold.pbtxt";
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
                {"statistic", dni::Datum(3000.0)},
            },
            {
                {"statistic", dni::Datum(5000.0)},
            },
            {
                {"statistic", dni::Datum(11000.0)},
            },
            {
                {"statistic", dni::Datum(13000000.0)},
            },
            {
                {"statistic", dni::Datum(15000001.0)},
            },
        };
        std::string out = "score";
        gen.Loop(inputs, out);

        g->Finish();

        spdlog::info("main over");

        return 0;
}
