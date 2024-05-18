#include "dni/framework/framework.h"
#include "samples/tools/datumgen.h"
#include "spdlog/spdlog.h"

int main()
{
        spdlog::set_level(spdlog::level::trace);

        const std::string& proto = "samples/dni/tasks/core/testdata/sum.pbtxt";
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
                {"score_1", dni::Datum(0.8)},
                {"score_2", dni::Datum(0.1)},
                {"score_3", dni::Datum(0.4)},
                {"score_4", dni::Datum(1.0)},
            },
        };
        std::string out = "sum";
        gen.Loop(inputs, out);

        g->Finish();

        spdlog::info("main over");

        return 0;
}
