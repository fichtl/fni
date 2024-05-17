#include <chrono>
#include <thread>

#include "dni/framework/framework.h"
#include "spdlog/spdlog.h"


void inject_after(dni::Graph* g, int after, int n, int interval)
{
        std::this_thread::sleep_for(std::chrono::milliseconds(after));

        double_t countTotal = 13000000;
        std::vector<double_t> countTotal_th = {4000, 10000, 100000, 15000000};
        std::vector<double_t> countTotal_score = {0, 0.4, 0.6, 0.8, 1.2};


        for (int i = 0; i < n; i++)
        {
                SPDLOG_DEBUG(
                    "send Datum({}) to graph g({:p})", dni::Datum(stat), fmt::ptr(g));

                // countTotal
                g->AddDatumToInputStream("countTotal", dni::Datum(countTotal));
                g->AddDatumToInputSideData("countTotal_th", dni::Datum(countTotal_th));
                g->AddDatumToInputSideData("countTotal_score", dni::Datum(countTotal_score));
                
                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        }
}

int main()
{
        spdlog::set_level(spdlog::level::trace);

        const std::string& proto = R"pb(
                type: "Anomaly Detection Graph"
                
                # packet: countTotal
                input_stream: "GIn_CountTotal:0:countTotal"
                input_side_data: "GIn_CountTotal_TH:0:countTotal_th"            
                input_side_data: "GIn_CountTotal_Score:0:countTotal_score"

                # Graph output
                output_stream: "NOut_packet_countTotal:0:packet_countTotal"
               
                node{
                        name: "packet_countTotal_score"
                        task: "ThresholdTask"
                        input_stream: "GIn_CountTotal:0:countTotal"
                        input_side_data: "GIn_CountTotal_TH:0:countTotal_th"            
                        input_side_data: "GIn_CountTotal_Score:0:countTotal_score" 
                        output_stream: "NOut_packet_countTotal:0:packet_countTotal"
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

        std::string out = "packet_countTotal";
        spdlog::debug("Create ObserveOutputStream: {}", out);

        g->ObserveOutputStream(out);
        spdlog::info("main 1");

        g->PrepareForRun();
        spdlog::info("main 2");

        inject_after(g, 0, 1, 0);
        spdlog::info("main 3");

        g->RunOnce();

        g->Wait();

        auto ret = g->GetResult<double_t>(out);
        spdlog::info("Gout {} result is: {}", out, ret);

        g->Finish();

        spdlog::info("main over");

        return 0;
}
