#include <chrono>
#include <thread>
#include <vector>
#include <fstream>
#include <string>
#include "samples/dni/tasks/snding_test/testenv.h"
#include "dni/framework/framework.h"
#include "spdlog/spdlog.h"

void inject_after(dni::Graph* g, double dataset, const std::vector<double_t>& condset, int after, int interval)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(after));

    SPDLOG_DEBUG("send Datum({}) to graph g({:p})", dni::Datum(dataset), fmt::ptr(g));
    g->AddDatumToInputStream("statistic", dni::Datum(dataset));
    g->AddDatumToInputStream("cond_values", dni::Datum(condset));

    std::this_thread::sleep_for(std::chrono::milliseconds(interval));   
}

struct TestData {
    // std::vector<uint64_t> test_thresholds;
    // std::vector<double_t> test_scores;
    // std::vector<std::vector<uint64_t>> cond_thresholds;  
    std::vector<double> test_stat;
    std::vector<std::vector<double_t>> test_cond;
    std::vector<double> test_result;
};

std::string out = "score";
std::string logFileName = "cond-threshold.log";

void testEachType(const TestData& data, dni::Graph* g, std::ofstream& outFile) {

    for (size_t i = 0; i < data.test_stat.size(); ++i) {
        const auto& stat = data.test_stat[i];
        const auto& cond = data.test_cond[i];
        outFile << "Test cond: ";
        for (uint64_t elem : data.test_cond[i]){
            outFile << elem << " ";
        }
        outFile << std::endl;
        // const auto& cond = data.test_cond[i];
        inject_after(g, stat, cond, 0, 200);
        spdlog::info("Test for dataset is {}", stat);

        g->RunOnce();
        g->Wait();

        auto ret = g->GetResult<double_t>(out);
        spdlog::info("Gout {} result is: {}", out, ret);

        double_t expected_sum = data.test_result[i];
        double_t error_margin = 1e-6; // 允许的误差范围
        if (std::abs(ret - expected_sum) < error_margin) {
            spdlog::info("Test for dataset {} is success ~ data: {}, Expected: {}, got: {}", i, stat, expected_sum, ret);
            outFile << "Test for dataset " << i << " is success ~ data: " << stat << " , Expected: " << expected_sum << ", got: " << ret << std::endl << std::endl;
        } else {
            spdlog::error("Test for dataset {} is fail ! data: {}, Expected: {}, got: {}", i, stat, expected_sum, ret);
            outFile << "Test for dataset " << i << " is fail ~ data:" << stat << " , Expected: " << expected_sum << ", got: " << ret << std::endl << std::endl;
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

    const std::string& proto = relatedpath + "testdata/cond_threshold.pbtxt";
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

    TestData TestData_length;
    //side
    // TestData_length.test_thresholds = {6, 500};
    // TestData_length.test_scores = {0.2, 0.1};
    // TestData_length.cond_thresholds = {
    //     {1000, 1500, 2000},
    //     {1000, 2500, 10000},
    //     {2000, 100, 2000},
    //     {3000, 9000, 5555, 1999},
    //     {3000, 9000, 5555, 2999},//5
    //     {3000, 9000, 5555, 2999},
    //     {3000, 9000, 5555, 1999},
    //     {1000, 1500, 25000}
    //     }; 
    //input
    TestData_length.test_stat = {4, 4, 4, 4, 7/*5*/, 500, 500, 500};
    TestData_length.test_cond = {
        {2000},
        {9000},
        {100},
        {2999},
        {2999},//5
        {2999},
        {2000},
        {1000}
        }; 
    TestData_length.test_result = {0.2, 0.2, 0, 0.2, 0, 0.1, 0.1, 0};
    //0508 run ok

    testEachType(TestData_length, g, outFile);

    g->Finish();
    spdlog::info("main over");

    outFile.close();

    return 0;
}