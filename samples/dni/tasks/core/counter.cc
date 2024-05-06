#include <chrono>
#include <thread>

#include "dni/framework/framework.h"
#include "spdlog/spdlog.h"

void inject_after(dni::Graph* g, int after, int n, int interval)
{
        std::this_thread::sleep_for(std::chrono::milliseconds(after));

        std::vector<double_t> sums = {0, 1, 1};
        double_t ___specified_number = 1;

        g->AddDatumToInputSideData(
                    "specified_number", dni::Datum(___specified_number));

        for (int i = 0; i < sums.size(); i++)
        {
                g->AddDatumToInputStream("sums_"+ std::to_string(i+1), dni::Datum(sums[i]));
        }
}

int main()
{
        spdlog::set_level(spdlog::level::trace);

        const std::string& proto = R"pb(
                type: "Counter"

                input_side_data: "SPECIFIED_NUMBER:0:specified_number"

                input_stream: "GIn_1:0:sums_1"
                input_stream: "GIn_2:0:sums_2"
                input_stream: "GIn_3:0:sums_3"

                output_stream: "GOut:0:count"

                node {
                  name: "A"
                  task: "CounterTask"
                  input_stream: "GIn_1:0:sums_1"
                  input_stream: "GIn_2:0:sums_2"
                  input_stream: "GIn_3:0:sums_3"

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