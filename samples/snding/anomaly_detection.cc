#include <chrono>
#include <cstdlib>
#include <thread>

#include "dni/framework/framework.h"
#include "spdlog/spdlog.h"

void inject_after(dni::Graph* g, int i, int j, int k)
{
        g->AddDatumToInputStream("packets", dni::Datum(double_t(i)));
        g->AddDatumToInputStream("netdev", dni::Datum(double_t(j)));
        g->AddDatumToInputStream("resource", dni::Datum(double_t(k)));
}

int main()
{
        spdlog::set_level(spdlog::level::trace);

        const std::string& proto = R"pb(
                type: "AnomalyDetection"

                input_stream: "GIn_Packets:0:packets"
                input_stream: "GIn_Netdev:0:netdev"
                input_stream: "GIn_Resource:0:resource"

                output_stream: "GOut:0:type"

                node {
                  name: "A"
                  task: "SndAdTask"

                  input_stream: "GIn_Packets:0:packets"
                  input_stream: "GIn_Netdev:0:netdev"
                  input_stream: "GIn_Resource:0:resource"

                  output_stream: "GOut:0:type"
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

        std::string out = "type";

        spdlog::debug("Create ObserveOutputStream: {}", out);
        g->ObserveOutputStream(out);

        g->PrepareForRun();

        for (int i = 0; i < 2; i++)
        {
                for (int j = 0; j < 2; j++)
                {
                        for (int k = 0; k < 2; k++)
                        {
                                inject_after(g, i, j, k);

                                g->RunOnce();

                                g->Wait();

                                auto ret = g->GetResult<int>(out);
                                spdlog::info("Gout {} result is: {}", out, ret);

                                g->ClearResult();
                        }
                }
        }

        g->Finish();

        spdlog::info("main over");

        return 0;
}
