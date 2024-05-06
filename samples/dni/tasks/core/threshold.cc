#include <chrono>
#include <thread>

#include "dni/framework/framework.h"
#include "spdlog/spdlog.h"

void inject_after(dni::Graph* g, int after, int n, int interval)
{
        std::this_thread::sleep_for(std::chrono::milliseconds(after));

        double_t stat = 13000000;
        std::vector<double_t> ___thresholds = {4000, 10000, 100000, 15000000};
        std::vector<double_t> ___scores = {0, 0.4, 0.6, 0.8, 1.2};

        for (int i = 0; i < n; i++)
        {
                SPDLOG_DEBUG(
                    "send Datum({}) to graph g({:p})", dni::Datum(stat), fmt::ptr(g));
                g->AddDatumToInputStream("statistic", dni::Datum(stat));

                g->AddDatumToInputSideData("thresholds", dni::Datum(___thresholds));
                g->AddDatumToInputSideData("scores", dni::Datum(___scores));

                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        }
}

int main()
{
        spdlog::set_level(spdlog::level::trace);

        const std::string& proto = R"pb(
                type: "Score"

                input_side_data: "THRESHOLD_VALUES:0:thresholds"
                input_side_data: "SCORE_VALUES:0:scores"
                input_stream: "GIn:0:statistic"
                output_stream: "GOut:0:score"

                node {
                  name: "A"
                  task: "ThresholdTask"
                  input_stream: "GIn:0:statistic"
                  input_side_data: "THRESHOLD_VALUES:0:thresholds"
                  input_side_data: "SCORE_VALUES:0:scores"
                  output_stream: "GOut:0:score"
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

        std::string out = "score";

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
