#include <string>
#include <unordered_map>
#include <vector>

#include "dni/framework/framework.h"
#include "samples/tools/datumgen.h"
#include "spdlog/spdlog.h"

int main()
{
        spdlog::set_level(spdlog::level::trace);

        const std::string& proto =
            "samples/dni/tasks/snding/testdata/protocol_stats.pbtxt";
        auto gc = dni::LoadTextprotoFile(proto);
        if (!gc)
        {
                spdlog::error("invalid pbtxt config: {}", proto);
                return -1;
        }

        dni::Graph* g = new dni::Graph(gc.value());
        DatumGen gen = DatumGen<double_t>(g);

        std::vector<InputMap> inputs = {
            {{"count", dni::Datum(std::unordered_map<uint32_t, int>{
                           {1, 6001}, {6, 2001}, {17, 1998}})}},
            {{"count", dni::Datum(std::unordered_map<uint32_t, int>{
                           {1, 3000}, {6, 3000}, {17, 4000}})}},
        };
        std::string out = "score";
        gen.Loop(inputs, out);

        g->Finish();

        spdlog::info("main over");

        return 0;
}
