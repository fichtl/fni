#include <chrono>
#include <string>
#include <thread>
#include <vector>

#include "dni/framework/framework.h"
#include "spdlog/spdlog.h"

void inject_after(dni::Graph* g, int after, int n, int interval)
{
        std::this_thread::sleep_for(std::chrono::milliseconds(after));

        double_t countTotal = 13000000;

        double_t countType_length = 499;
        std::vector<double_t> cond_countType_length = {2001};

        for (int i = 0; i < n; i++)
        {
                // countTotal
                g->AddDatumToInputStream("countTotal", dni::Datum(countTotal));

                // countType_length
                g->AddDatumToInputStream(
                    "countType_length", dni::Datum(countType_length));
                g->AddDatumToInputStream(
                    "cond_countType_length", dni::Datum(cond_countType_length));

                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        }
}

int main()
{
        spdlog::set_level(spdlog::level::trace);

        const std::string& proto = "samples/ddos/graph1/testdata/graph1_b.pbtxt";
        auto gc = dni::LoadTextprotoFile(proto);
        if (!gc)
        {
                spdlog::error("invalid pbtxt config: {}", proto);
                return -1;
        }

        dni::Graph* g = new dni::Graph(gc.value());

        std::string out = "packet_countTotal";
        spdlog::debug("Create ObserveOutputStream: {}", out);
        g->ObserveOutputStream(out);

        std::string out1 = "packet_countType_length";
        spdlog::debug("Create ObserveOutputStream: {}", out1);
        g->ObserveOutputStream(out1);

        g->PrepareForRun();

        inject_after(g, 0, 1, 0);

        g->RunOnce();
        g->Wait();
        auto ret = g->GetResult<double_t>(out);
        spdlog::info("Gout {} result is: {}", out, ret);

        auto ret1 = g->GetResult<double_t>(out1);
        spdlog::info("Gout {} result is: {}", out1, ret1);

        g->Finish();

        spdlog::info("main over");

        return 0;
}
