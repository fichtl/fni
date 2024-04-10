#include <chrono>
#include <thread>

#include "dni/framework/framework.h"
#include "spdlog/spdlog.h"

void inject_after(
    dni::Graph* g, dni::Datum&& d, dni::Datum&& sided, int after, int n, int interval)
{
        std::this_thread::sleep_for(std::chrono::milliseconds(after));

        for (int i = 0; i < n; i++)
        {
                SPDLOG_DEBUG("send Datum({}) to graph g({:p})", d, fmt::ptr(g));
                g->AddDatumToInputStream("GInSSS", d);

                g->AddDatumToInputSideData("extra_number", sided);

                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        }
}

int main()
{
        spdlog::set_level(spdlog::level::trace);

        const std::string& proto = R"pb(
                type: "Counter"

                input_side_data: "GSDIn:0:extra_number"
                input_stream: "GIn:0:GInSSS"
                output_stream: "GOut:0:GOutSSS"

                node {
                  name: "A"
                  task: "SideDataOutputTask"
                  input_stream: "GIn:0:GInSSS"
                  input_side_data: "GSDIn:0:extra_number"
                  output_stream: "AOut0:0:AOut0"
                  output_side_data: "ASDOut0:0:environment"
                }

                node {
                  name: "B"
                  task: "ProcessSideDataTask"
                  input_stream: "AOut0:0:AOut0"
                  input_side_data: "GSDIn:0:extra_number"
                  input_side_data: "ASDOut0:0:environment"
                  output_stream: "BOut:0:BOut"
                }

                node {
                  name: "C"
                  task: "ProcessSideDataTask"
                  input_stream: "BOut:0:BOut"
                  input_side_data: "GSDIn:0:extra_number"
                  input_side_data: "ASDOut0:0:environment"
                  output_stream: "COut:0:COut"
                }

                node {
                  name: "D"
                  task: "ProcessSideDataTask"
                  input_stream: "COut:0:COut"
                  input_side_data: "GSDIn:0:extra_number"
                  input_side_data: "ASDOut0:0:environment"
                  output_stream: "GOut:0:GOutSSS"
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

        std::string out = "GOutSSS";

        spdlog::debug("Create ObserveOutputStream: {}", out);
        g->ObserveOutputStream(out);

        g->PrepareForRun();

        inject_after(g, dni::Datum(10), dni::Datum(100), 0, 1, 0);

        g->RunOnce();

        g->Wait();

        int ret = g->GetResult<int>(out);
        spdlog::info("Gout result is: {:d}", ret);   // 207232227

        g->Finish();

        spdlog::info("main over");

        return 0;
}