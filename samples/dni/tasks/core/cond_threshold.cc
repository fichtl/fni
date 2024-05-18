#include "dni/framework/framework.h"
#include "samples/tools/datumgen.h"
#include "spdlog/spdlog.h"

int main()
{
        spdlog::set_level(spdlog::level::trace);

        const std::string& proto = "samples/dni/tasks/core/testdata/cond_threshold.pbtxt";
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
                {"statistic", dni::Datum(0.)},
                {"cond_values", dni::Datum(std::vector<double_t>{2000})},
            },
            {
                {"statistic", dni::Datum(6.)},
                {"cond_values", dni::Datum(std::vector<double_t>{1999})},
            },
            {
                {"statistic", dni::Datum(10.)},
                {"cond_values", dni::Datum(std::vector<double_t>{2000})},
            },
            {
                {"statistic", dni::Datum(500.)},
                {"cond_values", dni::Datum(std::vector<double_t>{1999})},
            },
            {
                {"statistic", dni::Datum(501.)},
                {"cond_values", dni::Datum(std::vector<double_t>{2000})},
            },
        };
        std::string out = "score";
        gen.Loop(inputs, out);

        g->Finish();

        spdlog::info("main over");

        return 0;
}
