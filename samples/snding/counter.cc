#include <chrono>
#include <thread>

#include "dni/framework/framework.h"
#include "spdlog/spdlog.h"

void inject_after(
    dni::Graph* g, int after, int n, int interval)
{
        std::this_thread::sleep_for(std::chrono::milliseconds(after));

        std::vector<uint64_t> sums = {0, 1, 1};
        uint64_t ___specified_number = 0;

        for (int i = 0; i < n; i++)
        {
                SPDLOG_DEBUG("send Datum({}) to graph g({:p})", dni::Datum(stat), fmt::ptr(g));
                g->AddDatumToInputStream("sums", dni::Datum(sums));

                g->AddDatumToInputSideData("specified_number", dni::Datum(___specified_number));

                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        }
}

int main()
{
        spdlog::set_level(spdlog::level::trace);

        const std::string& proto = R"pb(
                type: "Counter"

                input_side_data: "SPECIFIED_NUMBER:0:specified_number"
                input_stream: "GIn:0:sums"
                output_stream: "GOut:0:count"

                node {
                  name: "A"
                  task: "CounterTask"
                  input_stream: "GIn:0:sums"
                  input_side_data: "SPECIFIED_NUMBER:0:specified_number"
                  output_stream: "GOut:0:count"
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

        std::string out = "count";

        spdlog::debug("Create ObserveOutputStream: {}", out);
        g->ObserveOutputStream(out);

        g->PrepareForRun();

        inject_after(g, 0, 1, 0);

        g->RunOnce();

        g->Wait();

        auto ret = g->GetResult<uint64_t>(out);
        spdlog::info("Gout {} result is: {}", out, ret);

        g->Finish();

        spdlog::info("main over");

        return 0;
}
