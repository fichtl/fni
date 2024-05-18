#include <string>
#include <unordered_map>
#include <vector>

#include "dni/framework/framework.h"
#include "samples/tools/datumgen.h"
#include "spdlog/spdlog.h"

int main()
{
        spdlog::set_level(spdlog::level::trace);

        const std::string& proto = "samples/dni/tasks/snding/testdata/number_stats.pbtxt";
        auto gc = dni::LoadTextprotoFile(proto);
        if (!gc)
        {
                spdlog::error("invalid pbtxt config: {}", proto);
                return -1;
        }

        dni::Graph* g = new dni::Graph(gc.value());
        DatumGen gen = DatumGen<double_t>(g);

        std::unordered_map<uint32_t, int> count;
        for (size_t i = 0; i < 100; i++)
        {
                count[rand() % 100000] = 1000;
        }
        std::vector<InputMap> inputs = {
            {{"count", dni::Datum(count)}},
        };
        std::string out = "score";
        gen.Loop(inputs, out);

        g->Finish();

        spdlog::info("main over");

        return 0;
}
