#include <chrono>
#include <thread>

#include "dni/framework/framework.h"
#include "spdlog/spdlog.h"

void inject_after(dni::Graph* g, int after, int n, int interval)
{
        std::this_thread::sleep_for(std::chrono::milliseconds(after));

        std::unordered_map<uint32_t, int> count;
        count[1] = 6001;
        count[6] = 2001;
        count[17] = 1998;

        int ___protoCountSum = 10000;
        float_t ___ratioMin = 0.1;
        float_t ___ratioMax = 0.6;
        std::vector<float_t> ___score_thresholds = {0.8, 0.6, 0};

        for (int i = 0; i < n; i++)
        {
                // SPDLOG_DEBUG("send Datum({}) to graph g({:p})", d, fmt::ptr(g));
                g->AddDatumToInputStream("count", dni::Datum(count));

                g->AddDatumToInputSideData("protoCountSum", dni::Datum(___protoCountSum));
                g->AddDatumToInputSideData("ratioMin", dni::Datum(___ratioMin));
                g->AddDatumToInputSideData("ratioMax", dni::Datum(___ratioMax));
                g->AddDatumToInputSideData(
                    "score_thresholds", dni::Datum(___score_thresholds));

                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        }
}

void inject_after1(dni::Graph* g, int after, int n, int interval)
{
        std::this_thread::sleep_for(std::chrono::milliseconds(after));

        std::unordered_map<uint32_t, int> count;
        count[1] = 3000;
        count[6] = 3000;
        count[17] = 4000;

        int ___protoCountSum = 10000;
        float_t ___ratioMin = 0.1;
        float_t ___ratioMax = 0.6;
        std::vector<float_t> ___score_thresholds = {0.8, 0.6, 0};

        for (int i = 0; i < n; i++)
        {
                // SPDLOG_DEBUG("send Datum({}) to graph g({:p})", d, fmt::ptr(g));
                g->AddDatumToInputStream("count", dni::Datum(count));

                g->AddDatumToInputSideData("protoCountSum", dni::Datum(___protoCountSum));
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
                type: "ProtocolStats"

                input_side_data: "GSD_ProtoCountSum:0:protoCountSum"
                input_side_data: "GSD_RatioMin:0:ratioMin"
                input_side_data: "GSD_RatioMax:0:ratioMax"
                input_side_data: "GSD_ScoreThresholds:0:score_thresholds"
                input_stream: "COUNT:0:count"
                output_stream: "G_Score:0:score"

                node {
                  name: "A"
                  task: "ProtocolStatsTask"
                  input_side_data: "GSD_ProtoCountSum:0:protoCountSum"
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

        inject_after1(g, 0, 1, 0);

        g->RunOnce();

        g->Wait();

        auto ret = g->GetResult<float_t>(out);
        spdlog::info("Gout result is: {}", ret);

        g->Finish();

        spdlog::info("main over");

        return 0;
}
