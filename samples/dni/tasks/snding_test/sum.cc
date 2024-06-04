#include <chrono>
#include <thread>
#include <vector>
#include <fstream>
#include "samples/dni/tasks/snding_test/testenv.h"
#include "dni/framework/framework.h"
#include "spdlog/spdlog.h"

void inject_after(dni::Graph* g, const std::vector<double_t>& datasets, int after, int interval)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(after));
    SPDLOG_DEBUG("send Datum({}) to graph g({:p})", dni::Datum(datasets), fmt::ptr(g));
    for (size_t i = 0; i < datasets.size(); ) {
            std::ostringstream stream;
            double data = datasets[i];
            stream << "score_" << ++i;
            std::string datumName = stream.str();
            spdlog::info("[--------- data--------- {} ]:  {}", datumName, data);               
            g->AddDatumToInputStream(datumName, dni::Datum(data));  
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(interval));
}

int main()
{
    spdlog::set_level(spdlog::level::trace);
    std::string logFileName = "sum.log";   
    std::ofstream outFile(logFileName, std::ios::trunc);   

    if (!outFile.is_open()) {
        std::cerr << "Unable to open log file" << std::endl;
        return -1;
    } 

    const std::string& proto = relatedpath + "testdata/sum.pbtxt";
    auto gc = dni::LoadTextprotoFile(proto);
    if (!gc)
    {
            spdlog::error("invalid pbtxt config: {}", proto);
            return -1;
    }
    spdlog::debug(
        "GraphConfig: {}, node size: {}", gc.value().type(), gc.value().node_size());
    dni::Graph* g = new dni::Graph(gc.value());

    std::string out = "sum";
    // spdlog::debug("Create ObserveOutputStream: {}", out);
    g->ObserveOutputStream(out);
    g->PrepareForRun();

    std::vector<std::vector<double_t>> test_datasets = {
        {0.2, 0.2, 0.5, 0.1},//num 1
        {0.8, 0.7, 0.3, 0.5},
        {0.7, 0.3, 0.5, 0.6},
        {0.0, 0.3, 0.2, 0.1},
        {0.0, 0.6, 0.3, 0.5},
        {0.6, 0.1, 0.1, 0.1},//num 6
        {0.6, 0.4, 0.4, 1.0},
        {0.1, 0.5, 0.8, 1.0},
        {1.0, 0.1, 0.8, 0.1},
        {0.6, 0.7, 0.5, 0.8},
        
        {0.3, 0.7, 0.0, 0.1},//num 11
        {0.5, 0.4, 0.2, 0.4},
        {0.5, 1.0, 0.7, 0.7},
        {0.8, 0.3, 0.4, 0.5},
        {0.5, 0.4, 0.3, 0.1},
        {0.2, 0.2, 0.5, 0.3},//num 16
        {0.9, 0.5, 0.9, 0.4},
        {0.7, 0.8, 0.9, 0.6},
        {0.9, 0.9, 1.0, 0.7},
        {0.5, 0.1, 0.4, 0.1},
        {0.5, 0.4, 0.5, 0.8},//num 21
        {0.1, 1.0, 0.1, 0.5},
        {0.3, 0.4, 0.9, 0.3},
        {0.7, 0.3, 0.6, 0.8},
        {0.2, 0.3, 0.6, 0.3},
    };

    std::vector<double_t> test_sum = {
    1.0, 2.3, 2.1, 0.6, 1.4,
    0.8999999999999999, 2.4, 2.4, 2.0, 2.5999999999999996,
    1.1, 1.5, 2.9000000000000004, 2.0, 1.3,
    1.2, 2.6999999999999997, 3.0, 3.5, 1.1,
    2.2, 1.7000000000000002, 1.9000000000000001, 2.4000000000000004, 1.4000000000000001
    };
    
    for (size_t i = 0; i < test_datasets.size(); ++i) {
        const auto& scores = test_datasets[i];
        inject_after(g, std::vector<double_t>{scores}, 0, 200);
        spdlog::info("Test for dataset is {} ", scores);

        g->RunOnce();
        g->Wait();

        auto ret = g->GetResult<double_t>(out);
        spdlog::info("Gout {} result is: {}", out, ret);

        double_t expected_sum = test_sum[i];
        double_t error_margin = 1e-6; // 允许的误差范围
        if (std::abs(ret - expected_sum) < error_margin) {
            spdlog::info("Test for dataset {} is success ~ Expected: {}, got: {}", i, expected_sum, ret);
            outFile << "Test for dataset " << i << " is success. Expected: " << expected_sum << ", got: " << ret << std::endl;
        } else {
            spdlog::error("Test for dataset {} is fail ! Expected: {}, got: {}", i, expected_sum, ret);
            outFile << "Test for dataset " << i << " is fail. Expected: " << expected_sum << ", got: " << ret << std::endl;
        }

        g->ClearResult();
    }

    g->Finish();
    spdlog::info("main over");
    outFile.close();

    return 0;
}