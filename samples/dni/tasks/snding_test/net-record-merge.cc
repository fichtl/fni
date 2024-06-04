#include <chrono>
#include <dirent.h>
#include <fstream>
#include <thread>
#include <vector>
#include <bitset>
#include <sstream>
#include <unistd.h>
#include <limits.h>
#include "dni/framework/framework.h"
#include "dni/tasks/snding/snding_defines.h"
#include "samples/dni/tasks/snding_test/testenv.h"
#include <spdlog/sinks/basic_file_sink.h>

#include "spdlog/spdlog.h"

void inject_after(dni::Graph* g, const std::string& datasets, const std::string& hostnic, int i, int after, int interval)
{
        std::this_thread::sleep_for(std::chrono::milliseconds(after));
        std::vector<uint32_t> ___known_ip = {};

        g->AddDatumToInputStream("pcap", dni::Datum(datasets));
        g->AddDatumToInputStream("known_ip", dni::Datum(___known_ip));

        g->AddDatumToInputStream(
            "host_nic_name", dni::Datum(hostnic));
        std::this_thread::sleep_for(std::chrono::milliseconds(interval));
}

void inject_after_graph2(dni::Graph* g, std::vector<std::unordered_map<std::string, dni::snding::SIPBaseMergeStats>>& datasets, int after, int interval)
{
        std::this_thread::sleep_for(std::chrono::milliseconds(after));       

        for (size_t i = 0; i < datasets.size(); ++i)
        {
                std::ostringstream stream;
                std::unordered_map<std::string, dni::snding::SIPBaseMergeStats> map = datasets[i];
                stream << "sip_merge" << i;
                std::string datumName = stream.str();
                spdlog::info("[--------- map--------- {} ]:  {}", datumName, map);               
                g->AddDatumToInputStream(datumName, dni::Datum(map));     
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(interval));
}

std::string extract_filename(const std::string& path) {
    size_t last_slash_idx = path.rfind('/');
    size_t last_dot_idx = path.rfind('.');
    if (last_slash_idx == std::string::npos || last_dot_idx == std::string::npos) {
        return ""; 
    }
    std::string filename = path.substr(last_slash_idx + 1, last_dot_idx - last_slash_idx - 1);
    return filename;
}

int main()
{
        spdlog::set_level(spdlog::level::trace);
        std::string logFileName = "net-record-merge.log";
        std::string spdlogFileName = "net-record-merge-spd.log";       
        std::ofstream outFile(logFileName, std::ios::trunc);   
        
        if (!outFile.is_open())
        {
                std::cerr << "Unable to open log file" << std::endl << std::endl;
                return -1;
        }
        auto test_logger =
            spdlog::basic_logger_mt(" ", spdlogFileName);
        std::ofstream file(spdlogFileName, std::ofstream::out | std::ofstream::trunc);
        file.close();
        
        test_logger->set_level(spdlog::level::trace);
        test_logger->flush_on(spdlog::level::trace);

        const std::string& proto = relatedpath + "testdata/net_record_merge_graph1.pbtxt";
        
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
        std::string out8 = "sip_dms_rules_prepare";
        spdlog::debug("Create ObserveOutputStream: {}", out);
        spdlog::debug("Create ObserveOutputStream1: {}", out1);
        spdlog::debug("Create ObserveOutputStream2: {}", out2);
        spdlog::debug("Create ObserveOutputStream3: {}", out3);
        spdlog::debug("Create ObserveOutputStream4: {}", out4);
        spdlog::debug("Create ObserveOutputStream5: {}", out5);
        spdlog::debug("Create ObserveOutputStream6: {}", out6);
        spdlog::debug("Create ObserveOutputStream6: {}", out7);
        spdlog::debug("Create ObserveOutputStream6: {}", out8);
        g->ObserveOutputStream(out);
        g->ObserveOutputStream(out1);
        g->ObserveOutputStream(out2);
        g->ObserveOutputStream(out3);
        g->ObserveOutputStream(out4);
        g->ObserveOutputStream(out5);
        g->ObserveOutputStream(out6);
        g->ObserveOutputStream(out7);
        g->ObserveOutputStream(out8);
        g->PrepareForRun();

        std::vector<std::string> test_datasets;

        DIR* dir;
        struct dirent* entry;
        char full_path[PATH_MAX];
        const char* testdir = testmerge;
        spdlog::info("Testdir: {} ", testdir);
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
        std::vector<std::unordered_map<std::string, dni::snding::SIPBaseMergeStats>> net_att_info;

        for (size_t i = 0; i < test_datasets.size(); ++i)
        {
                const auto& tests = test_datasets[i];
                std::string filename = extract_filename(tests);
                inject_after(g, tests, filename, i, 0, 200);
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

                auto ret6 = g->GetResult<dni::snding::AttackerIPMergeResult>(out6);
                spdlog::info("Gout {} result is: {}", out6, ret6);
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
                }

                auto ret7 = g->GetResult<std::unordered_map<std::string, dni::snding::SIPBaseMergeStats>>(out7);
                spdlog::info("Gout {} result is: {}", out7, ret7);  
                net_att_info.push_back(ret7);

                auto ret8 = g->GetResult<std::unordered_map<std::string, std::unordered_map<std::string, std::string>>>(out8);
                spdlog::info("Gout {} result is: {}", out8, ret8); 
                outFile << "sip_dms_rules_prepare: " << std::endl;
                for (const auto& outerPair : ret8) {
                    outFile << "pre-rule: " << outerPair.first << ",  ";     
                    for (const auto& innerPair : outerPair.second) {
                        outFile << innerPair.first << ": " << innerPair.second << ",  ";
                    }
                    outFile << std::endl;
                }
                outFile << std::endl;

                test_logger->info("-------------------------------test {} --------------------------------", i);
                test_logger->info("[ Dataset ]:  {}", tests);
                test_logger->info("[ SIP ]:  {}", ret);
                test_logger->info("[ Sport ]:  {}", ret1);
                test_logger->info("[ Dport ]:  {}", ret2);
                test_logger->info("[ Length ]:  {}", ret3);
                test_logger->info("[ Protocol ]:  {}", ret4);
                test_logger->info("[ Max (sport, dport, length) ]:  {}", ret5);
                test_logger->info("[ Attacker ip merge ]:  \n{}", ret6);          
                test_logger->info("[ Sip base merge ]:  \n{}", ret7);           
                test_logger->info("[ sip_dms_rules_prepare ]:  \n{}", ret8);   
               
                g->ClearResult();
        }
        g->Finish();
        //--------------------------------------graph2----------------------------------------------
        
        const std::string& proto1 = relatedpath + "testdata/net_record_merge_graph2.pbtxt";

        auto gc1 = dni::LoadTextprotoFile(proto1);
        if (!gc1)
        {
                spdlog::error("invalid pbtxt config: {}", proto1);
                return -1;
        }
        spdlog::debug(
            "GraphConfig: {}, node size: {}", gc1.value().type(), gc1.value().node_size());

        dni::Graph* g1 = new dni::Graph(gc1.value());

        std::string out2_1 = "attack_link";

        spdlog::debug("Create ObserveOutputStream: {}", out2_1);
        g1->ObserveOutputStream(out2_1);

        g1->PrepareForRun();

        inject_after_graph2(g1, net_att_info, 1, 0);

        g1->RunOnce();

        g1->Wait();

        auto ret2_1 = g1->GetResult<
            std::unordered_map<std::string, std::unordered_set<std::string>>>(out2_1);
        outFile << "-------------------------------- graph 2 ------------------------------------" << std::endl;
        outFile << "net_record_merge: " << std::endl;
        for (const auto& outerPair : ret2_1) {
                outFile << "attack_link: " << outerPair.first << ":  ";     
                for (const auto& value  : outerPair.second) {
                outFile << value << ";  ";
                }
                outFile << std::endl;
        }
        spdlog::info("Gout {} result size is: {}", out2_1, ret2_1.size());
        spdlog::info("Gout {} result is: {}", out2_1, ret2_1);

        g1->Finish();
        test_logger->info("\n--------------------------------graph 2------------------------------------\n");
        test_logger->info("[ net record merge size ]: {}", ret2_1.size());
        test_logger->info("[ net record merge ]: \n{}", ret2_1);

        spdlog::info("main over");

        outFile.close();

        return 0;
}
