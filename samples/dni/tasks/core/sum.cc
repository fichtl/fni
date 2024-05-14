#include <chrono>
#include <thread>

#include "dni/framework/framework.h"
#include "spdlog/spdlog.h"

void inject_after(dni::Graph* g, int after, int n, int interval)
{
        std::this_thread::sleep_for(std::chrono::milliseconds(after));

        std::vector<double_t> scores = {0.8, 0.1, 0.4, 1};

        for (int i = 0; i < scores.size(); i++)
        {
                g->AddDatumToInputStream(
                    "score_" + std::to_string(i + 1), dni::Datum(scores[i]));
        }
}

int main()
{
        spdlog::set_level(spdlog::level::trace);

        const std::string& proto = R"pb(
                type: "Sum"

                input_stream: "GIn_1:0:score_1"
                input_stream: "GIn_2:0:score_2"
                input_stream: "GIn_3:0:score_3"
                input_stream: "GIn_4:0:score_4"

                output_stream: "GOut:0:sum"

                node {
                  name: "A"
                  task: "SumTask"
                  input_stream: "GIn_1:0:score_1"
                  input_stream: "GIn_2:0:score_2"
                  input_stream: "GIn_3:0:score_3"
                  input_stream: "GIn_4:0:score_4"

                  output_stream: "GOut:0:sum"
                }
        )pb";

        auto gc = dni::ParseStringToGraphConfig(proto);
        if (!gc)
        {
                spdlog::error("invalid pbtxt config: {}", proto);
                return -1;
        }
        spdlog::debug(
            "GraphConfig: {}, node size: {}", gc.value().type(), gc.value().node_size());

        dni::Graph* g = new dni::Graph(gc.value());

        std::string out = "sum";

        spdlog::debug("Create ObserveOutputStream: {}", out);
        g->ObserveOutputStream(out);

        g->PrepareForRun();

        inject_after(g, 0, 1, 0);

        g->RunOnce();

        g->Wait();

        auto ret = g->GetResult<double_t>(out);
        spdlog::info("Gout {} result is: {}", out, ret);

        g->Finish();

        spdlog::info("main over");

        return 0;
}
