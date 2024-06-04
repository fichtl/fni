#include <chrono>
#include <dirent.h>
#include <fstream>
#include <thread>
#include <vector>
#include <bitset>
#include "dni/framework/framework.h"
#include "dni/tasks/snding/snding_defines.h"
#include "samples/dni/tasks/snding_test/testenv.h"
#include <spdlog/sinks/basic_file_sink.h>

#include "spdlog/spdlog.h"

void inject_after(dni::Graph* g, const std::string& datasets, const std::string& hostnic, int after, int interval)
{
        std::this_thread::sleep_for(std::chrono::milliseconds(after));
        std::vector<uint32_t> ___known_ip = {};

        g->AddDatumToInputStream("pcap", dni::Datum(datasets));
        g->AddDatumToInputStream("known_ip", dni::Datum(___known_ip));

        g->AddDatumToInputStream(
            "host_nic_name", dni::Datum(hostnic));
        std::this_thread::sleep_for(std::chrono::milliseconds(interval));
}

std::string extract_filename(const std::string& path) {
    size_t last_slash_idx = path.rfind('/');
    size_t last_dot_idx = path.rfind('-');
    if (last_slash_idx == std::string::npos || last_dot_idx == std::string::npos) {
        return ""; 
    }
    std::string filename = path.substr(last_slash_idx + 1, last_dot_idx - last_slash_idx - 1);
    return filename;
}

int main()
{
        // spdlog::set_level(spdlog::level::trace);
        std::string logFileName = "sip-base-merge.log";
        std::string spdlogFileName = "sip-base-merge-spd.log";
        
        std::ofstream outFile(logFileName, std::ios::trunc);   
      
        if (!outFile.is_open())
        {
                std::cerr << "Unable to open log file" << std::endl;
                return -1;
        }
        auto test_logger =
            spdlog::basic_logger_mt(" ", spdlogFileName);
        std::ofstream file(spdlogFileName, std::ofstream::out | std::ofstream::trunc);
        file.close();
        
        test_logger->set_level(spdlog::level::trace);
        test_logger->flush_on(spdlog::level::trace);

        const std::string& proto = relatedpath + "testdata/sip_base_merge.pbtxt";

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
        std::string out6 = "parsedips";
        std::string out7 = "sip_cidr_based_packets_merge";
        spdlog::debug("Create ObserveOutputStream: {}", out);
        spdlog::debug("Create ObserveOutputStream1: {}", out1);
        spdlog::debug("Create ObserveOutputStream2: {}", out2);
        spdlog::debug("Create ObserveOutputStream3: {}", out3);
        spdlog::debug("Create ObserveOutputStream4: {}", out4);
        spdlog::debug("Create ObserveOutputStream5: {}", out5);
        spdlog::debug("Create ObserveOutputStream6: {}", out6);
        spdlog::debug("Create ObserveOutputStream6: {}", out7);
        g->ObserveOutputStream(out);
        g->ObserveOutputStream(out1);
        g->ObserveOutputStream(out2);
        g->ObserveOutputStream(out3);
        g->ObserveOutputStream(out4);
        g->ObserveOutputStream(out5);
        g->ObserveOutputStream(out6);
        g->ObserveOutputStream(out7);
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

        // std::vector<uint64_t> test_result = {0, 2, 1, 1, 1, 1, 2, 0};

        for (size_t i = 0; i < test_datasets.size(); ++i)
        {
                const auto& tests = test_datasets[i];
                std::string filename = extract_filename(tests);
                inject_after(g, tests, filename, 0, 200);
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

                test_logger->info("-------------------------------test {} --------------------------------", i);
                test_logger->info("[ Dataset ]:  {}", tests);
                test_logger->info("[ SIP ]:  {}", ret);
                test_logger->info("[ Sport ]:  {}", ret1);
                test_logger->info("[ Dport ]:  {}", ret2);
                test_logger->info("[ Length ]:  {}", ret3);
                test_logger->info("[ Protocol ]:  {}", ret4);
                test_logger->info("[ Max (sport, dport, length) ]:  {}", ret5);

                auto ret6 = g->GetResult<dni::snding::AttackerIPMergeResult>(out6);
                
                for (const auto& ip : ret6.attackerIPs)
                {
                        spdlog::info("CIDR IP is: {}", ip);
                        outFile << "CIDR IP: " << std::hex << ip.ip << ", Length: " << std::dec << ip.len
                                << std::endl;
                }
                if (ret6.containRandomAttack)
                {
                        outFile << "containRandomAttack: " << ret6.containRandomAttack
                                << std::endl
                                << "Random IP: ";
                        int j = 1;
                        for (uint32_t ip : ret6.randomIPs)
                        {
                                spdlog::info("Random IP is: {}", ip);
                                if (j % 10 == 0)
                                {
                                        outFile << std::endl;
                                }
                                outFile << std::hex << ip << " , ";
                                j++;
                                if (j == 100)
                                {
                                        outFile << "......";
                                        break;
                                }
                        }
                        outFile << std::endl;
                }
                else
                {
                        outFile << "containRandomAttack: " << ret6.containRandomAttack
                                << std::endl;
                        spdlog::info("Gout {} result is: {}", out6, ret6);
                        test_logger->info("[ Attacker ip merge ]:  \n{}", ret6); 
                }

                auto ret7 = g->GetResult<std::unordered_map<std::string, dni::snding::SIPBaseMergeStats>>(out7);
                spdlog::info("Gout {} result is: {}", out7, ret7);  
                

                         
                test_logger->info("[ Sip base merge ]:  \n{}", ret7);           

                outFile << std::endl;

                g->ClearResult();
        }

        g->Finish();
        spdlog::info("main over");

        outFile.close();

        return 0;
}
