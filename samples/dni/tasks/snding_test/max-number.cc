#include <chrono>
#include <thread>
#include <vector>
#include <fstream>
#include <dirent.h>
#include "samples/dni/tasks/snding_test/testenv.h"
#include "dni/framework/framework.h"
#include "spdlog/spdlog.h"

void inject_after(dni::Graph* g, const std::string& datasets, int after, int interval)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(after));

    SPDLOG_DEBUG("send Datum({}) to graph g({:p})", dni::Datum(datasets), fmt::ptr(g));
    g->AddDatumToInputStream("pcap", dni::Datum(datasets));

    std::this_thread::sleep_for(std::chrono::milliseconds(interval));
    
}

int main()
{
    spdlog::set_level(spdlog::level::trace);
    std::string logFileName = "max-number.log";   
    std::ofstream outFile(logFileName, std::ios::trunc);  
    
    if (!outFile.is_open()) {
        std::cerr << "Unable to open log file" << std::endl;
        return -1;
    } 

    const std::string& proto =  relatedpath + "testdata/max_number.pbtxt";

    auto gc = dni::LoadTextprotoFile(proto);
        if (!gc)
        {
                spdlog::error("invalid pbtxt config: {}", proto);
                return -1;
        }
        spdlog::debug(
            "GraphConfig: {}, node size: {}", gc.value().type(), gc.value().node_size());
        dni::Graph* g = new dni::Graph(gc.value());

    std::string out = "score";
    std::string out1 = "score1";
    std::string out2 = "score2";
    std::string out3 = "score3";
    std::string out4 = "score4";
    std::string out5 = "max";
    spdlog::debug("Create ObserveOutputStream: {}", out);
    spdlog::debug("Create ObserveOutputStream1: {}", out1);
    spdlog::debug("Create ObserveOutputStream2: {}", out2);
    spdlog::debug("Create ObserveOutputStream3: {}", out3);
    spdlog::debug("Create ObserveOutputStream4: {}", out4);
    spdlog::debug("Create ObserveOutputStream5: {}", out5);
    g->ObserveOutputStream(out);
    g->ObserveOutputStream(out1);
    g->ObserveOutputStream(out2);
    g->ObserveOutputStream(out3);
    g->ObserveOutputStream(out4);
    g->ObserveOutputStream(out5);
    g->PrepareForRun();

    std::vector<std::string> test_datasets; 

    DIR* dir;
    struct dirent* entry;
    char full_path[PATH_MAX];

    dir = opendir(testdir);
    if (dir == NULL)
    {
            perror("opendir() error");
            return 1;
    }

    while ((entry = readdir(dir)) != NULL)
    {                
            if (entry->d_type == DT_REG)
            {
                    snprintf(full_path, sizeof(full_path),  "%s%s", testdir, entry->d_name);
                    test_datasets.push_back(full_path);
            }
    }
    closedir(dir);

    std::vector<uint64_t> test_result = {0, 2, 1, 1, 1, 1, 2, 0};

    for (size_t i = 0; i < test_datasets.size(); ++i) {
        const auto& tests = test_datasets[i];
        
        inject_after(g, tests, 0, 200);
        spdlog::info("Test for dataset is {} ", tests);
        outFile << i << ": Dataset [ " << tests << " ]" << std::endl;

        g->RunOnce();
        g->Wait();
        
        auto ret = g->GetResult<double_t>(out);
        spdlog::info("Gout SIP result is: {}", ret);
        outFile << "Gout SIP result is: " << ret << std::endl;

        auto ret1 = g->GetResult<double_t>(out1);
        spdlog::info("Gout sport result is: {}", ret1);
        outFile << "Gout sport result is: " << ret1 << std::endl; 

        auto ret2 = g->GetResult<double_t>(out2);
        spdlog::info("Gout dport result is: {}", ret2);
        outFile << "Gout dport result is: " << ret2 << std::endl; 

        auto ret3 = g->GetResult<double_t>(out3);
        spdlog::info("Gout length result is: {}", ret3);
        outFile << "Gout length result is: " << ret3 << std::endl; 

        auto ret4 = g->GetResult<double_t>(out4);
        spdlog::info("Gout Protocol result is: {}", ret4);
        outFile << "Gout Protocol result is: " << ret4 << std::endl; 

        auto ret5 = g->GetResult<double_t>(out5);
        spdlog::info("Gout max[sport,dport,length] result is: {}", ret5);
        outFile << "Gout max[sport,dport,length] result is: " << ret5 << std::endl;         

        outFile << std::endl;

        g->ClearResult();
    }

    g->Finish();
    spdlog::info("main over");

    outFile.close();

    return 0;
}
