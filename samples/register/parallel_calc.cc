#include <chrono>
#include <thread>

#include "dni/framework/datum.h"
#include "dni/framework/graph.h"
#include "fmt/format.h"
#include "spdlog/spdlog.h"

void inject_after(dni::Graph* g, dni::Datum&& d, int after, int interval)
{
        std::this_thread::sleep_for(std::chrono::milliseconds(after));
        spdlog::debug("d: {}", d);
        g->AddDatumToInputStream("GInSSS", d);
        g->AddDatumToInputStream("GInTTT", d);
        std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        spdlog::debug("inject_after exit");
}
int main()
{
        spdlog::set_level(spdlog::level::trace);

        std::string proto = R"pb(
                type: "TwoWayCounter"

                input_stream: "GIn0:0:GInSSS"
                input_stream: "GIn1:0:GInTTT"

                output_stream: "GOut0:0:GOutSSS"
                output_stream: "GOut1:0:GOutTTT"
                output_stream: "GOut2:0:GOutUUU"
                output_stream: "GOut3:0:GOutVVV"

                node {
                  name: "A"
                  task: "AddOrMulTask"
                  input_stream: "GIn0:0:GInSSS"
                  input_stream: "GIn1:0:GInTTT"
                  output_stream: "AOut0:0:AOut000"
                  output_stream: "AOut1:0:AOut111"
                }

                node {
                  name: "B"
                  task: "SubOrDivTask"
                  input_stream: "AOut0:0:AOut000"
                  input_stream: "AOut1:0:AOut111"
                  output_stream: "BOut0:0:BOut000"
                  output_stream: "BOut1:0:BOut111"
                }

                node {
                  name: "C"
                  task: "AddOrMulTask"
                  input_stream: "BOut0:0:BOut000"
                  input_stream: "BOut1:0:BOut111"
                  output_stream: "COut0:0:COut000"
                  output_stream: "COut1:0:COut111"
                }

                node {
                  name: "D"
                  task: "SubOrDivTask"
                  input_stream: "COut0:0:COut000"
                  input_stream: "COut1:0:COut111"
                  output_stream: "GOut0:0:GOutSSS"
                  output_stream: "GOut1:0:GOutTTT"
                }
        )pb";

        auto gc = dni::ParseStringToGraphConfig(proto);
        if (!gc.has_value())
        {
                spdlog::error("invalid pbtxt config: {}", proto);
                return -1;
        }

        spdlog::debug(
            "GraphConfig: {}, node size: {}", gc.value().type(), gc.value().node_size());

        dni::Graph* g = new dni::Graph(gc.value());

        std::string out1 = std::string("GOutSSS");
        g->ObserveOutputStream(out1);
        spdlog::debug("Create ObserveOutputStream: {}", out1);

        std::string out2 = std::string("GOutTTT");
        g->ObserveOutputStream(out2);
        spdlog::debug("Create ObserveOutputStream: {}", out2);

        g->PrepareForRun();

        for (int i = 0; i < 5; i++)
        {
                inject_after(g, dni::Datum(10 + i), 0, 1000);

                g->RunOnce();

                g->Wait();

                auto ret = g->GetResult<int>(out1);
                spdlog::info("Gout:{} result is: {:d}", out1, ret);

                auto ret2 = g->GetResult<int>(out2);
                spdlog::info("Gout:{} result is: {:d}", out2, ret2);

                g->ClearResult();
        }

        g->Finish();

        spdlog::info("main over");

        return 0;
}
