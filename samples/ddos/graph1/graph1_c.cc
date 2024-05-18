#include <chrono>
#include <thread>
#include <vector>

#include "dni/framework/framework.h"
#include "spdlog/spdlog.h"

void inject_after(dni::Graph* g, int after, int n, int interval)
{
        std::this_thread::sleep_for(std::chrono::milliseconds(after));

        // packet: countTotal
        double_t countTotal = 13000000;

        // packet: countType_length
        double_t countType_length = 2;
        std::vector<double_t> cond_countType_length = {2001};

        // packet: countType_srcIP
        double_t countType_srcIP = 3;
        std::vector<double_t> cond_countType_srcIP = {2001};

        // packet: countType_dstIP
        double_t countType_dstIP = 1;
        std::vector<double_t> cond_countType_dstIP = {2001};

        // packet: countType_protocol
        double_t countType_protocol = 1;
        std::vector<double_t> cond_countType_protocol = {2001};

        // packet: countType_srcPortTcp
        double_t countType_srcPortTcp = 1;
        std::vector<double_t> cond_countType_srcPortTcp = {2001};

        // packet: countType_dstPortTcp
        double_t countType_dstPortTcp = 1;
        std::vector<double_t> cond_countType_dstPortTcp = {2001};

        // packet: countType_srcPortUdp
        double_t countType_srcPortUdp = 1;
        std::vector<double_t> cond_countType_srcPortUdp = {2001};

        // packet: countType_dstPortUdp
        double_t countType_dstPortUdp = 1;
        std::vector<double_t> cond_countType_dstPortUdp = {2001};

        // netdev: inMbps
        double_t inMbps = 1024.22;

        // netdev: inKpps
        double_t inKpps = 10000;

        // netdev: outMbps
        double_t outMbps = 10;

        // netdev: outKpps
        double_t outKpps = 1;

        // resource: cur_cpu
        double_t cur_cpu = 30;

        // resource: incr_mem_swap
        double_t incr_mem_swap = 1;

        // resource: incr_mem_virtual
        double_t incr_mem_virtual = 1;

        // resource: incr_mem_full
        double_t incr_mem_full = 1;

        // resource: incr_tcpconn_semi
        double_t incr_tcpconn_semi = 1;

        // resource: incr_tcpconn_total
        double_t incr_tcpconn_total = 1;

        for (int i = 0; i < n; i++)
        {
                // packet: countTotal
                g->AddDatumToInputStream("countTotal", dni::Datum(countTotal));

                // packet: countType_length
                g->AddDatumToInputStream(
                    "countType_length", dni::Datum(countType_length));
                g->AddDatumToInputStream(
                    "cond_countType_length", dni::Datum(cond_countType_length));

                // packet: countType_srcIP
                g->AddDatumToInputStream("countType_srcIP", dni::Datum(countType_srcIP));
                g->AddDatumToInputStream(
                    "cond_countType_srcIP", dni::Datum(cond_countType_srcIP));

                // packet: countType_dstIP
                g->AddDatumToInputStream("countType_dstIP", dni::Datum(countType_dstIP));
                g->AddDatumToInputStream(
                    "cond_countType_dstIP", dni::Datum(cond_countType_dstIP));

                // packet: countType_protocol
                g->AddDatumToInputStream(
                    "countType_protocol", dni::Datum(countType_protocol));
                g->AddDatumToInputStream(
                    "cond_countType_protocol", dni::Datum(cond_countType_protocol));

                // packet: countType_srcPortTcp
                g->AddDatumToInputStream(
                    "countType_srcPortTcp", dni::Datum(countType_srcPortTcp));
                g->AddDatumToInputStream(
                    "cond_countType_srcPortTcp", dni::Datum(cond_countType_srcPortTcp));

                // packet: countType_dstPortTcp
                g->AddDatumToInputStream(
                    "countType_dstPortTcp", dni::Datum(countType_dstPortTcp));
                g->AddDatumToInputStream(
                    "cond_countType_dstPortTcp", dni::Datum(cond_countType_dstPortTcp));

                // packet: countType_srcPortUdp
                g->AddDatumToInputStream(
                    "countType_srcPortUdp", dni::Datum(countType_srcPortUdp));
                g->AddDatumToInputStream(
                    "cond_countType_srcPortUdp", dni::Datum(cond_countType_srcPortUdp));

                // packet: countType_dstPortUdp
                g->AddDatumToInputStream(
                    "countType_dstPortUdp", dni::Datum(countType_dstPortUdp));
                g->AddDatumToInputStream(
                    "cond_countType_dstPortUdp", dni::Datum(cond_countType_dstPortUdp));

                // netdev: inMbps
                g->AddDatumToInputStream("inMbps", dni::Datum(inMbps));

                // netdev: inKpps
                g->AddDatumToInputStream("inKpps", dni::Datum(inKpps));

                // netdev: outMbps
                g->AddDatumToInputStream("outMbps", dni::Datum(outMbps));

                // netdev: outKpps
                g->AddDatumToInputStream("outKpps", dni::Datum(outKpps));

                // resource: cur_cpu
                g->AddDatumToInputStream("cur_cpu", dni::Datum(cur_cpu));

                // resource: incr_mem_swap
                g->AddDatumToInputStream("incr_mem_swap", dni::Datum(incr_mem_swap));

                // resource: incr_mem_virtual
                g->AddDatumToInputStream(
                    "incr_mem_virtual", dni::Datum(incr_mem_virtual));

                // resource: incr_mem_full
                g->AddDatumToInputStream("incr_mem_full", dni::Datum(incr_mem_full));

                // resource: incr_tcpconn_semi
                g->AddDatumToInputStream(
                    "incr_tcpconn_semi", dni::Datum(incr_tcpconn_semi));

                // resource: incr_tcpconn_total
                g->AddDatumToInputStream(
                    "incr_tcpconn_total", dni::Datum(incr_tcpconn_total));

                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        }
}

int main()
{
        spdlog::set_level(spdlog::level::trace);

        const std::string& proto = "samples/ddos/graph1/testdata/graph1_c.pbtxt";
        auto gc = dni::LoadTextprotoFile(proto);
        if (!gc)
        {
                spdlog::error("invalid pbtxt config: {}", proto);
                return -1;
        }

        dni::Graph* g = new dni::Graph(gc.value());

        std::string out00 = "packet_scores_sum";
        spdlog::debug("Create ObserveOutputStream: {}", out00);
        g->ObserveOutputStream(out00);

        std::string out01 = "netdev_scores_sum";
        spdlog::debug("Create ObserveOutputStream: {}", out01);
        g->ObserveOutputStream(out01);

        std::string out02 = "resource_scores_sum";
        spdlog::debug("Create ObserveOutputStream: {}", out02);
        g->ObserveOutputStream(out02);

        std::string out10 = "packet_abnormal";
        spdlog::debug("Create ObserveOutputStream: {}", out10);
        g->ObserveOutputStream(out10);

        std::string out11 = "netdev_abnormal";
        spdlog::debug("Create ObserveOutputStream: {}", out11);
        g->ObserveOutputStream(out11);

        std::string out12 = "resource_abnormal";
        spdlog::debug("Create ObserveOutputStream: {}", out12);
        g->ObserveOutputStream(out12);

        std::string out2 = "abnormal_res";
        spdlog::debug("Create ObserveOutputStream: {}", out2);
        g->ObserveOutputStream(out2);

        g->PrepareForRun();
        inject_after(g, 0, 1, 0);
        g->RunOnce();
        g->Wait();

        auto ret00 = g->GetResult<double_t>(out00);
        spdlog::info("Gout {} result is: {}", out00, ret00);

        auto ret01 = g->GetResult<double_t>(out01);
        spdlog::info("Gout {} result is: {}", out01, ret01);

        auto ret02 = g->GetResult<double_t>(out02);
        spdlog::info("Gout {} result is: {}", out02, ret02);

        auto ret10 = g->GetResult<double_t>(out10);
        spdlog::info("Gout {} result is: {}", out10, ret10);

        auto ret11 = g->GetResult<double_t>(out11);
        spdlog::info("Gout {} result is: {}", out11, ret11);

        auto ret12 = g->GetResult<double_t>(out12);
        spdlog::info("Gout {} result is: {}", out12, ret12);

        auto ret2 = g->GetResult<int>(out2);
        spdlog::info("Gout {} result is: {}", out2, ret2);

        g->Finish();

        spdlog::info("main over");

        return 0;
}
