#include <chrono>
#include <thread>
#include <vector>
#include <fstream>

#include "dni/framework/framework.h"
#include "spdlog/spdlog.h"

void inject_after(dni::Graph* g, const std::vector<double_t>& datasets, int after, int interval)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(after));
    SPDLOG_DEBUG("send Datum({}) to graph g({:p})", dni::Datum(datasets), fmt::ptr(g));
    g->AddDatumToInputStream("scores", dni::Datum(datasets));
    std::this_thread::sleep_for(std::chrono::milliseconds(interval));
}

int main()
{
    spdlog::set_level(spdlog::level::trace);
    std::string logFileName = "sum-public.log";   
    std::ofstream outFile(logFileName, std::ios::trunc);    
    if (!outFile.is_open()) {
        std::cerr << "Unable to open log file" << std::endl;
        return -1;
    } 

    const std::string& proto = R"pb(
            type: "Sum"

            input_stream: "GIn:0:scores"
            output_stream: "GOut:0:sum"

            node {
                name: "A"
                task: "SumTask"
                input_stream: "GIn:0:scores"
                output_stream: "GOut:0:sum"
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

    std::string out = "sum";
    // spdlog::debug("Create ObserveOutputStream: {}", out);
    g->ObserveOutputStream(out);
    g->PrepareForRun();

    std::vector<std::vector<double_t>> test_datasets = {
        {0.14, 0.48, 0.3, 0.79, 0.8, 0.71, 0.51, 0.97, 0.97, 0.94, 0.16},//num 1
        {0.13, 0.63, 0.05, 0.8, 0.43, 0.89, 0.99, 0.35, 0.54, 0.5, 0.46, 0.74, 0.23, 0.92, 0.21, 0.67, 0.57, 0.16},
        {0.16, 0.45, 0.28, 0.49, 0.86, 0.78, 0.65, 0.37, 0.63, 0.67, 0.21, 0.12, 0.25, 0.53},
        {0.62, 0.77, 0.03, 0.4, 0.55},
        {0.12, 0.12, 0.51, 0.94, 0.99, 0.93, 0.49, 0.56, 0.16, 0.03},
        {0.93, 0.52, 0.41, 0.49, 0.07, 0.01, 0.26, 0.81, 0.34, 0.51},//num 6
        {0.8, 0.13, 0.38, 0.23, 0.09, 0.91, 0.17, 0.8, 0.89, 0.03, 0.42, 0.3, 0.89, 0.76, 0.97, 0.72, 0.69, 0.47, 1.0, 0.36},
        {0.05, 0.8, 0.99, 0.37, 0.87, 0.02, 0.55, 0.94, 0.73, 0.99, 0.85, 0.2, 0.72, 0.52, 0.04, 0.63, 0.54, 0.09, 0.51},
        {0.83, 0.32, 0.94, 0.24, 0.47},
        {0.47, 0.25, 0.38, 0.68, 0.27, 0.71, 0.28, 0.18, 0.76, 0.24, 0.88, 0.93, 0.43, 0.96, 0.07, 0.86, 0.52, 0.48, 0.0, 0.21},
        
        {1.78, 8.92, 5.55},//num 11
        {5.97, 2.17, 7.16, 2.73, 2.84, 7.67, 3.64, 5.73, 0.12, 5.65, 9.58, 0.78, 1.54, 2.81, 1.72},
        {7.16, 2.9},
        {7.81, 7.24, 8.92, 8.92, 9.2, 8.38, 9.43, 9.99, 5.68, 9.45},
        {0.35, 2.71, 6.35, 8.9, 5.26, 5.98, 3.81, 1.67, 5.37, 7.24, 8.0, 5.16, 8.05, 7.03, 9.92, 9.54, 3.91},
        {8.135, 3.03, 3.201, 8.858, 8.19, 0.201, 8.518, 0.963, 0.753, 5.728, 6.39, 7.761, 7.951, 7.239, 4.142, 1.672, 0.793, 1.774},//num 16
        {4.281, 8.758, 9.137, 9.345, 3.15, 0.844, 9.063, 6.493, 2.041},
        {8.376, 0.348, 8.377, 3.431, 8.992, 3.909, 6.288, 2.768, 5.04, 6.114, 5.137, 6.83, 3.408, 3.263, 4.825},
        {2.667, 2.952, 1.602},
        {4.957, 9.495, 7.566, 2.99, 0.93, 9.832, 8.958},
        {-7.9, -3.3, -9.0, 3.3, -6.6, 0.9, -1.5, 7.0},//num 20
        {-8.8, 8.0, 4.6, 1.7, -1.4, -10.0, 2.1, -0.0, 6.6, 6.0, -1.4, -2.1, -9.8, 1.1},
        {6.7, -1.0, 6.7, -3.4, 7.2, 8.1, 7.1, 9.7, 3.1, 9.1, -7.9},
        {-0.5, -9.5, -9.4, 0.5, 1.8, -1.2},
        {-4.7, -7.2, 7.8, -6.6, 3.5, 8.7, 3.7, 0.9, 2.2, -3.9, -2.9},
    };

    std::vector<double_t> test_sum = {
    6.77, 9.270000000000001, 6.449999999999999, 2.37, 4.8500000000000005,
    4.35, 11.009999999999998, 10.409999999999998, 2.8, 9.56,
    16.25, 60.10999999999999, 10.06, 85.02, 99.25,
    85.299, 53.112, 77.10600000000001, 7.221, 44.727999999999994,
    -17.1, -3.4000000000000017, 45.4, -18.299999999999997, 1.5000000000000004
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