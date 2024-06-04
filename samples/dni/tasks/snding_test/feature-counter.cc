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
    std::vector<std::string> feats = {"SIP", "SPort", "DPort", "Protocol", "Length"};

    SPDLOG_DEBUG("send Datum({}) to graph g({:p})", dni::Datum(datasets), fmt::ptr(g));
    g->AddDatumToInputStream("pcap", dni::Datum(datasets));
    std::this_thread::sleep_for(std::chrono::milliseconds(interval));
    
}

int main()
{
    spdlog::set_level(spdlog::level::trace);
    std::string logFileName = "feature-counter.log";
    std::ofstream outFile(logFileName, std::ios::trunc); 
    
    if (!outFile.is_open()) {
        std::cerr << "Unable to open log file" << std::endl;
        return -1;
    } 

    const std::string& proto = relatedpath + "testdata/feature_counter.pbtxt";

    auto gc = dni::LoadTextprotoFile(proto);
    if (!gc)
    {
            spdlog::error("invalid pbtxt config: {}", proto);
            return -1;
    }
    spdlog::debug(
        "GraphConfig: {}, node size: {}", gc.value().type(), gc.value().node_size());
    dni::Graph* g = new dni::Graph(gc.value());

    std::vector<std::string> out_stream_names = {
        "sip_count", "sport_count", "dport_count", "protocol_count", "length_count"};

    for (const auto& name : out_stream_names)
    {
            spdlog::debug("Create ObserveOutputStream: {}", name);
            g->ObserveOutputStream(name);
    }
    
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
        const auto& scores = test_datasets[i];
        
        inject_after(g, scores, 0, 200);
        spdlog::info("Test for dataset is {} ", scores);
        outFile << i << ": Dataset [ " << scores << " ]" << std::endl;

        g->RunOnce();
        g->Wait();
        
        for (const auto& name : out_stream_names)
        {
            auto ret = g->GetResult<std::unordered_map<uint32_t, int>>(name);
            int j = 1;
            spdlog::info("Gout {} result is: {}", name, ret);
            outFile << name << std::endl;
            int all=0;
            int numkeylen = ret.size();
            for (const auto& pair : ret) { 
                // outFile << j << ") "; 
                outFile << pair.first << ": " << pair.second << "  ";
                if (j % 10 == 0) { outFile << std::endl; }
                all = all + pair.second;
                j++;
            } 
            outFile << std::endl;   
            outFile << name << "_all: " << all << "    numkeylen: " << numkeylen << std::endl;
        }
        outFile << std::endl;   

        g->ClearResult();
    }

    g->Finish();
    spdlog::info("main over");

    outFile.close();

    return 0;
}