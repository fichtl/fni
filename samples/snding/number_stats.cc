#include <chrono>
#include <thread>

#include "dni/framework/framework.h"
#include "spdlog/spdlog.h"

void inject_after(dni::Graph* g, int after, int n, int interval)
{
        std::this_thread::sleep_for(std::chrono::milliseconds(after));

        srand(time(NULL));

        std::unordered_map<uint32_t, int> count;
        for (size_t i = 0; i < 100; i++)
        {
                count[rand() % 100000] = 1000;
        }

        int ___numValueSum = 10000;
        double_t ___ratioMin = 0.1;
        double_t ___ratioMax = 0.6;
        std::vector<double_t> ___score_thresholds = {0.4, 1.0, 0.6, 0};

        for (int i = 0; i < n; i++)
        {
                // SPDLOG_DEBUG("send Datum({}) to graph g({:p})", d, fmt::ptr(g));
                g->AddDatumToInputStream("count", dni::Datum(count));

                g->AddDatumToInputSideData("numValueSum", dni::Datum(___numValueSum));
                g->AddDatumToInputSideData("ratioMin", dni::Datum(___ratioMin));
                g->AddDatumToInputSideData("ratioMax", dni::Datum(___ratioMax));
                g->AddDatumToInputSideData(
                    "score_thresholds", dni::Datum(___score_thresholds));

                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        }
}

int main()
{
        spdlog::set_level(spdlog::level::trace);

        const std::string& proto = R"pb(
                type: "SndNumberStats"

                input_side_data: "GSD_NumValueSum:0:numValueSum"
                input_side_data: "GSD_RatioMin:0:ratioMin"
                input_side_data: "GSD_RatioMax:0:ratioMax"
                input_side_data: "GSD_ScoreThresholds:0:score_thresholds"
                input_stream: "COUNT:0:count"
                output_stream: "G_Score:0:score"

                node {
                  name: "A"
                  task: "SndNumberStatsTask"
                  input_side_data: "GSD_NumValueSum:0:numValueSum"
                  input_side_data: "GSD_RatioMin:0:ratioMin"
                  input_side_data: "GSD_RatioMax:0:ratioMax"
                  input_side_data: "GSD_ScoreThresholds:0:score_thresholds"
                  input_stream: "COUNT:0:count"
                  output_stream: "G_Score:0:score"
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
        spdlog::info("Gout result is: {}", ret);

        g->Finish();

        spdlog::info("main over");

        return 0;
}
