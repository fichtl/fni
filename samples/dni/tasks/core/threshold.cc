#include <chrono>
#include <thread>

#include "dni/framework/framework.h"
#include "spdlog/spdlog.h"

void inject_after(dni::Graph* g, int after, int interval)
{
        std::this_thread::sleep_for(std::chrono::milliseconds(after));

        std::vector<double_t> stats = {3000, 5000, 11000, 13000000, 15000001};

        for (double_t stat : stats)
        {
                SPDLOG_DEBUG(
                    "send Datum({}) to graph g({:p})", dni::Datum(stat), fmt::ptr(g));
                g->AddDatumToInputStream("statistic", dni::Datum(stat));

                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        }
}

int main()
{
        spdlog::set_level(spdlog::level::trace);

        const std::string& proto = R"pb(
                type: "Score"

                input_stream: "GIn:0:statistic"
                output_stream: "GOut:0:score"

                node {
                  name: "A"
                  task: "ThresholdTask"
                  input_stream: "GIn:0:statistic"
                  output_stream: "GOut:0:score"

                  options {
                    [type.asnapis.io/dni.ThresholdTaskOptions] {
                      thresh_scores { threshold: 4000 score: 0.4}
                      thresh_scores { threshold: 10000 score: 0.6}
                      thresh_scores { threshold: 100000 score: 0.8}
                      thresh_scores { threshold: 15000000 score: 1.2}
                    }
                  }
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

        std::vector<double_t> stats = {3000, 5000, 11000, 13000000, 15000001};
        for (double_t& stat : stats)
        {
                spdlog::info("Input: {}", stat);
                SPDLOG_DEBUG(
                    "send Datum({}) to graph g({:p})", dni::Datum(stat), fmt::ptr(g));
                g->AddDatumToInputStream("statistic", dni::Datum(stat));

                g->RunOnce();

                g->Wait();

                auto ret = g->GetResult<double_t>(out);
                spdlog::info("Gout {} result is: {}", out, ret);
        }

        g->Finish();

        spdlog::info("main over");

        return 0;
}
