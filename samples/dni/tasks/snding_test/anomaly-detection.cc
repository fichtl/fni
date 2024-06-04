#include <chrono>
#include <thread>
#include <vector>
#include <fstream>
#include "samples/dni/tasks/snding_test/testenv.h"
#include "dni/framework/framework.h"
#include "spdlog/spdlog.h"

void inject_after(dni::Graph* g, const std::vector<double>& datasets, int after, int interval)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(after));
    SPDLOG_DEBUG("send Datum({}) to graph g({:p})", dni::Datum(datasets), fmt::ptr(g));
    std::vector<std::string> datumNames = {"packets", "netdev", "resource"};
    for (size_t i = 0; i < datasets.size(); ++i) {
            double data = datasets[i];
            std::string datumName = datumNames[i];
            spdlog::debug("[--------- data---------i:{} {} ]:  {}", i, datumName, data);               
            g->AddDatumToInputStream(datumName, dni::Datum(data));  
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(interval));   
}

int main()
{
    spdlog::set_level(spdlog::level::trace);
    std::string logFileName = "anomaly-detection.log";
    std::ofstream outFile(logFileName, std::ios::trunc); 
    
    if (!outFile.is_open()) {
        std::cerr << "Unable to open log file" << std::endl;
        return -1;
    } 

    const std::string& proto = relatedpath + "testdata/anomaly_detection.pbtxt";
    auto gc = dni::LoadTextprotoFile(proto);
    if (!gc)
    {
            spdlog::error("invalid pbtxt config: {}", proto);
            return -1;
    }
    spdlog::debug(
        "GraphConfig: {}, node size: {}", gc.value().type(), gc.value().node_size());
    dni::Graph* g = new dni::Graph(gc.value());

    std::string out = "type";

    g->ObserveOutputStream(out);
    g->PrepareForRun();

    std::vector<std::vector<double>> test_datasets = {
       {0, 0, 0},
       {0, 0, 1}, 
       {0, 1, 0}, 
       {0, 1, 1}, 
       {1, 0, 0}, 
       {1, 0, 1}, 
       {1, 1, 0}, 
       {1, 1, 1} 
    };

    std::vector<int> test_result = {0, 0, 0, 4, 0, 3, 2, 1};

    for (size_t i = 0; i < test_datasets.size(); ++i) {
        const auto& scores = test_datasets[i];
        // const auto& num = test_num[i];
        inject_after(g, std::vector<double>{scores}, 0, 200);
        spdlog::info("Test for dataset is {} ", scores);
        outFile << i << ": Dataset [";
        for (double_t elem : scores){
            outFile << elem << " ";
        }
        outFile << "]" << std::endl;

        g->RunOnce();
        g->Wait();

        auto ret = g->GetResult<int>(out);
        spdlog::info("Gout {} result is: {}", out, ret);
        
        if (ret == test_result[i]) {
            spdlog::info("Test for dataset {} is success ~ Expected: {}, got: {}", i, test_result[i], ret);
            outFile << "Test for dataset " << i << " is success. Expected: " << test_result[i] << ", got: " << ret << std::endl << std::endl;
        } else {
            spdlog::error("Test for dataset {} is fail ! Expected: {}, got: {}", i, test_result[i], ret);
            outFile << "Test for dataset " << i << " is fail. Expected: " << test_result[i] << ", got: " << ret << std::endl << std::endl;;
        }

        g->ClearResult();
    }

    g->Finish();
    spdlog::info("main over");

    outFile.close();

    return 0;
}