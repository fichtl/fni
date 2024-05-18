#include "dni/framework/framework.h"
#include "samples/tools/datumgen.h"
#include "spdlog/spdlog.h"

int main()
{
        spdlog::set_level(spdlog::level::trace);

        const std::string& proto = "samples/dni/tasks/core/testdata/counter.pbtxt";
        auto gc = dni::LoadTextprotoFile(proto);
        if (!gc)
        {
                spdlog::error("invalid pbtxt config: {}", proto);
                return -1;
        }

        dni::Graph* g = new dni::Graph(gc.value());
        DatumGen gen = DatumGen<uint64_t>(g);

        std::vector<InputMap> inputs = {
            {
                {"sums_1", dni::Datum(0.)},
                {"sums_2", dni::Datum(1.)},
                {"sums_3", dni::Datum(1.)},
            },
        };
        std::string out = "count";
        gen.Loop(inputs, out);

        g->Finish();

        spdlog::info("main over");

        return 0;
}
