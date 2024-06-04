#include <chrono>
#include <thread>
#include <vector>
#include <fstream>
#include <string>
#include "samples/dni/tasks/snding_test/testenv.h"
#include "dni/framework/framework.h"
#include "spdlog/spdlog.h"

void inject_after(dni::Graph* g, double dataset, int after, int interval)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(after));
    SPDLOG_DEBUG("send Datum({}) to graph g({:p})", dni::Datum(dataset), fmt::ptr(g));
    g->AddDatumToInputStream("statistic", dni::Datum(dataset));
    std::this_thread::sleep_for(std::chrono::milliseconds(interval));
}

struct TestData {
    // std::vector<double> test_thresholds;
    // std::vector<double_t> test_scores;
    std::vector<double> test_stat;
    std::vector<double_t> test_result;
};

std::string out = "score";
std::string logFileName = "threshold.log";

void testEachType(const TestData& data, dni::Graph* g, std::ofstream& outFile) {
    // outFile << "Test scores: ";
    // for (double_t elem : data.test_scores){
    //     outFile << elem << " ";
    // }
    // outFile << std::endl;
    // outFile << "Test threshold: ";
    // for (double_t elem : data.test_thresholds){
    //     outFile << elem << " ";
    // }
    // outFile << std::endl;
    for (size_t i = 0; i < data.test_stat.size(); ++i) {
        const auto& stat = data.test_stat[i];
        inject_after(g, stat, 0, 200);
        spdlog::info("Test for dataset is {} ", stat);

        g->RunOnce();
        g->Wait();

        auto ret = g->GetResult<double_t>(out);
        spdlog::info("Gout {} result is: {}", out, ret);

        double_t expected_sum = data.test_result[i];
        double_t error_margin = 1e-6; 
        if (std::abs(ret - expected_sum) < error_margin) {
            spdlog::info("Test for dataset {} is success ~ data: {}, Expected: {}, got: {}", i, stat, expected_sum, ret);
            outFile << "Test for dataset " << i << " is success ~ data:" << stat << ",Expected: " << expected_sum << ", got: " << ret << std::endl;
        } else {
            spdlog::error("Test for dataset {} is fail ! data: {}, Expected: {}, got: {}", i, stat, expected_sum, ret);
            outFile << "Test for dataset " << i << " is fail ~ data:" << stat << ",Expected: " << expected_sum << ", got: " << ret << std::endl;
        }

        g->ClearResult();    
    }
}

int main()
{
    spdlog::set_level(spdlog::level::trace);
    std::ofstream outFile(logFileName, std::ios::trunc);  
    if (!outFile.is_open()) {
        std::cerr << "Unable to open log file" << std::endl;
        return -1;
    } 

    const std::string& proto = relatedpath + "testdata/threshold.pbtxt";
    auto gc = dni::LoadTextprotoFile(proto);
    if (!gc)
    {
            spdlog::error("invalid pbtxt config: {}", proto);
            return -1;
    }
    spdlog::debug(
        "GraphConfig: {}, node size: {}", gc.value().type(), gc.value().node_size());
    dni::Graph* g = new dni::Graph(gc.value());

    spdlog::debug("Create ObserveOutputStream: {}", out);
    g->ObserveOutputStream(out);
    g->PrepareForRun();

    TestData TestData_count;
    // TestData_count.test_thresholds = {4000, 10000, 100000, 15000000};
    // TestData_count.test_scores = {0, 0.4, 0.6, 0.8, 1.2};

    TestData_count.test_stat = {0, 1000, 4000, 6000, 10000, 99999, 100000, 14000000, 15000000, 150000000, 1500000000, 3999, 9999, 14999999};
    TestData_count.test_result = {0, 0, 0.4, 0.4, 0.6, 0.6, 0.8, 0.8, 1.2, 1.2, 1.2, 0, 0.4, 0.8};
    //
    TestData TestData_Mbps;
    // TestData_Mbps.test_thresholds = {100, 500, 2000000000, 15000000000};
    // TestData_Mbps.test_scores = {0, 0.2, 0.4, 0.6, 1};

    TestData_Mbps.test_stat = {100, 500, 1000000000, 2000000000, 15000000000};
    TestData_Mbps.test_result = {0.2, 0.4, 0.4, 0.6, 1};
    //
    TestData TestData_Kpps;
    // TestData_Kpps.test_thresholds = {4000, 10000, 100000, 15000000};
    // TestData_Kpps.test_scores = {0, 0.2, 0.4, 0.6, 1};

    TestData_Kpps.test_stat = {0, 1000, 4000, 6000, 10000, 99999, 100000, 14000000, 15000000, 150000000, 1500000000, 3999, 9999, 14999999};
    TestData_Kpps.test_result = {0, 0, 0.2, 0.2, 0.4, 0.4, 0.6, 0.6, 1, 1, 1, 0, 0.2, 0.6};
    // testEachType(TestData_Mbps, g, outFile);   
    testEachType(TestData_count, g, outFile);
    // testEachType(TestData_Kpps, g, outFile);

    g->Finish();
    spdlog::info("main over");

    outFile.close();

    return 0;
}