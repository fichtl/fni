#include <chrono>
#include <thread>

#include "dni/framework/framework.h"
#include "spdlog/spdlog.h"

void inject_after(dni::Graph* g, int after, int n, int interval)
{
        std::this_thread::sleep_for(std::chrono::milliseconds(after));

        double_t stat = 400;                             // length
        std::vector<double_t> ___cond_values = {2000};   // countTotal

        for (int i = 0; i < n; i++)
        {
                g->AddDatumToInputStream("cond_values", dni::Datum(___cond_values));
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
                type: "ConditionThreshold"

                input_stream: "GIn:0:statistic"
                input_stream: "GCondIn:0:cond_values"
                output_stream: "GOut:0:score"

                node {
                  name: "A"
                  task: "ConditionThresholdTask"
                  input_stream: "GIn:0:statistic"
                  input_stream: "GCondIn:0:cond_values"
                  output_stream: "GOut:0:score"

                  options {
                    [type.asnapis.io/dni.CondThresholdTaskOptions] {
                      conditions: 2000
                      thresh_scores { threshold: 6 score: 0.2}
                      thresh_scores { threshold: 500 score: 0.1}
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

        std::vector<double_t> stats = {0, 6, 10, 500, 501};
        std::vector<double_t> conds = {2000, 1999, 2000, 1999, 2000};
        for (int i = 0; i < stats.size(); i++)
        {
                spdlog::info("Input: stat:{}, conditions:[{}]", stats[i], conds[i]);
                SPDLOG_DEBUG(
                    "send Datum({}) to graph g({:p})", dni::Datum(stats[i]), fmt::ptr(g));
                g->AddDatumToInputStream("statistic", dni::Datum(stats[i]));
                g->AddDatumToInputStream(
                    "cond_values", dni::Datum(std::vector<double_t>{conds[i]}));

                g->RunOnce();

                g->Wait();

                auto ret = g->GetResult<double_t>(out);
                spdlog::info("Gout {} result is: {}", out, ret);
        }

        g->Finish();

        spdlog::info("main over");

        return 0;
}
