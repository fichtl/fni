#include <chrono>
#include <thread>

#include "dni/framework/framework.h"
#include "spdlog/spdlog.h"

void inject_after(dni::Graph* g, int after, int interval)
{
        std::this_thread::sleep_for(std::chrono::milliseconds(after));

        for (int i = 0; i < 1; i++)
        {
                int num = 10;
                dni::Datum d(num);

                SPDLOG_DEBUG("send Datum({}) to g({:p})", num, fmt::ptr(g));
                g->AddDatumToInputStream("GInSSS", d);

                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        }
}

int main()
{
        spdlog::set_level(spdlog::level::trace);

        auto gc = dni::ParseStringToGraphConfig(R"pb(
                type: "Counter"

                input_stream: "GIn:0:GInSSS"
                output_stream: "GOut:0:GOutSSS"

                node {
                  name: "A"
                  task: "CounterTask"
                  input_stream: "GIn:0:GInSSS"
                  output_stream: "AOut0:0:AOut0"
                }

                node {
                  name: "B"
                  task: "CounterTask"
                  input_stream: "AOut0:0:AOut0"
                  output_stream: "BOut:0:BOut"
                }

                node {
                  name: "C"
                  task: "CounterTask"
                  input_stream: "BOut:0:BOut"
                  output_stream: "COut:0:COut"
                }

                node {
                  name: "D"
                  task: "CounterTask"
                  input_stream: "COut:0:COut"
                  output_stream: "GOut:0:GOutSSS"
                }
        )pb");
        if (!gc.has_value())
                return -1;

        dni::Graph* g = new dni::Graph(gc.value());

        std::string out = std::string("GOutSSS");

        g->ObserveOutputStream(out);

        g->PrepareForRun();

        inject_after(g, 0, 0);

        // std::thread t1(inject_after, g, 3000, 1000);
        g->RunOnce();

        // t1.join();
        g->Wait();

        int ret = g->GetResult<const int>(out);
        spdlog::info("Gout result is: {:d}", ret);

        g->Finish();

        spdlog::info("main over");

        return 0;
}
