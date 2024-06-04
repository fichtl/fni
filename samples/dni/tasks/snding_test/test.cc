#include <iostream>
#include <string>
#include <vector>
#include <fstream>

int main() {
    std::string logFileName = "test_result.log";
    std::ofstream outFile(logFileName, std::ios::trunc);  
    
    if (!outFile.is_open()) {
        std::cerr << "Unable to open log file" << std::endl;
        return -1;
    } 
    const std::string directory = "samples/dni/tasks/snding_test"; 
    const std::vector<std::string> test_programs = {
        "cond-threshold", "sum", "anomaly-detection", "threshold", "counter",  
        "protocol-stats", "pcap-parse", "feature-counter", "number-stats", "max-number",
        "attacker-ip-merge", "sip-base-merge", "sip-base-merge-dedup", "net-record-merge", "dms-rule-dedup"
    };

    for (const std::string& test : test_programs) {
        std::string command = "cd " + directory + "; ./" + test;
        int result = system(command.c_str());
        if (result != 0) {
            outFile << "Test [" << test << "] Failed with exit code :" << result << std::endl;
        } else {
            outFile << "Test [" << test << "] Passed ~" << std::endl;
        }
    }

    return 0;
}

