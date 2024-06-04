
#ifndef TESTDIR_H
#define TESTDIR_H

inline const std::string relatedpath = "../../../../../../../../dni/samples/dni/tasks/snding_test/";

//pcap NIC name followed by '-' for sip_base_merge_task, sip_base_merge_dedup_task  
std::string pcapdir = relatedpath + "pcap/";
inline const char* testdir = pcapdir.c_str();

//for net_record_merge_task, dms_rules_dedup_task

std::string merge = relatedpath + "pcap/net_record_merge/3hping3-syn-rand/";
inline const char* testmerge = merge.c_str();


#endif // TESTDIR_H 