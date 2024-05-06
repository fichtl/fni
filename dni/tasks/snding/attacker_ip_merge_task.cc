#include <algorithm>
#include <list>
#include <vector>

#include "dni/framework/framework.h"
#include "snding_defines.h"
#include "dni/tasks/datagram/cidr.h"
#include "spdlog/spdlog.h"

namespace dni {

        class SndAttackerIPMergeTask: public TaskBase {
        public:
                SndAttackerIPMergeTask(): name_("SndAttackerIPMergeTask") {}
                ~SndAttackerIPMergeTask() override {}

                int Open(TaskContext* ctx) override
                {
                        SPDLOG_DEBUG("Task {}: open task ...", name_);

                        return 0;
                }

                int Process(TaskContext* ctx) override;

                int Close(TaskContext* ctx) override
                {
                        SPDLOG_DEBUG("Task {}: closing ...", name_);

                        return 0;
                }

        private:
                bool belongs_24(uint32_t cidr_24, uint32_t ip);
                bool belongs_16(uint32_t cidr_16, uint32_t ip);

                std::string name_;
        };

        int SndAttackerIPMergeTask::Process(TaskContext* ctx)
        {
                // input 0, key: srcip/32, value: count
                Datum ip_map_d = ctx->Inputs()[0].Value();
                SPDLOG_DEBUG("Task {}: Consume ip_map: {}", name_, ip_map_d);
                auto ip_map_opt = ip_map_d.Consume<std::unordered_map<uint32_t, int>>();
                if (!ip_map_opt)
                {
                        SPDLOG_WARN(
                            "Task {}: Consume() 'ip_map' returns NULL, wait for "
                            "input ...",
                            name_);
                }
                auto ipCountDF = *(ip_map_opt.value());
                SPDLOG_DEBUG("Task {}: val: {}", name_, ipCountDF);

                // input 1, all known IPs in net, /32
                Datum all_known_ips_d = ctx->Inputs()[1].Value();
                SPDLOG_DEBUG(
                    "Task {}: Consume all_known_ips: {}", name_, all_known_ips_d);
                auto all_known_ips_opt = all_known_ips_d.Consume<std::vector<uint32_t>>();
                if (!all_known_ips_opt)
                {
                        SPDLOG_WARN(
                            "Task {}: Consume() 'ip_map' returns NULL, wait for "
                            "input ...",
                            name_);
                }
                auto all_known_ips = *(all_known_ips_opt.value());
                SPDLOG_DEBUG("Task {}: val: {}", name_, all_known_ips);

                // input side data 0
                // double_t ipFw4CountRatio = 0.1;

                auto ipFw4CountRatio_d = ctx->GetInputSideData()[0];
                SPDLOG_DEBUG(
                        "Task {}: Consume side data ipFw4CountRatio: {}",
                        name_,
                        ipFw4CountRatio_d);

                auto ipFw4CountRatio_opt = ipFw4CountRatio_d.Consume<double_t>();
                if (!ipFw4CountRatio_opt)
                {
                        SPDLOG_WARN(
                                "Task {}: Consume() ipFw4CountRatio returns NULL, "
                                "wait for input ...",
                                name_);
                }

                auto ipFw4CountRatio = *(ipFw4CountRatio_opt.value());
                SPDLOG_DEBUG("Task {}: ipFw4CountRatio: {}", name_, ipFw4CountRatio);

                // input side data 1
                // double_t ipFw3CountRatio = 0.2;

                auto ipFw3CountRatio_d = ctx->GetInputSideData()[1];
                SPDLOG_DEBUG(
                        "Task {}: Consume side data ipFw3CountRatio: {}", name_, ipFw3CountRatio_d);

                auto ipFw3CountRatio_opt = ipFw3CountRatio_d.Consume<double_t>();
                if (!ipFw3CountRatio_opt)
                {
                        SPDLOG_WARN(
                                "Task {}: Consume() ipFw3CountRatio returns NULL, "
                                "wait for input ...",
                                name_);
                }

                auto ipFw3CountRatio = *(ipFw3CountRatio_opt.value());
                SPDLOG_DEBUG("Task {}: ipFw3CountRatio: {}", name_, ipFw3CountRatio);


                // input side data 2
                // int ipSegCoverThreshold = 100;

                auto ipSegCoverThreshold_d = ctx->GetInputSideData()[2];
                SPDLOG_DEBUG(
                        "Task {}: Consume side data ipSegCoverThreshold: {}", name_, ipSegCoverThreshold_d);

                auto ipSegCoverThreshold_opt = ipSegCoverThreshold_d.Consume<int>();
                if (!ipSegCoverThreshold_opt)
                {
                        SPDLOG_WARN(
                                "Task {}: Consume() ipSegCoverThreshold returns NULL, "
                                "wait for input ...",
                                name_);
                }

                auto ipSegCoverThreshold = *(ipSegCoverThreshold_opt.value());
                SPDLOG_DEBUG("Task {}: ipSegCoverThreshold: {}", name_, ipSegCoverThreshold);

                // input side data 3
                // double_t ipFw2CountRatio = 0.4;

                auto ipFw2CountRatio_d = ctx->GetInputSideData()[3];
                SPDLOG_DEBUG(
                        "Task {}: Consume side data ipFw2CountRatio: {}",
                        name_,
                        ipFw2CountRatio_d);

                auto ipFw2CountRatio_opt =
                        ipFw2CountRatio_d.Consume<double_t>();
                if (!ipFw2CountRatio_opt)
                {
                        SPDLOG_WARN(
                                "Task {}: Consume() ipFw2CountRatio returns NULL, "
                                "wait for input ...",
                                name_);
                }

                auto ipFw2CountRatio = *(ipFw2CountRatio_opt.value());
                SPDLOG_DEBUG(
                        "Task {}: ipFw2CountRatio: {}", name_, ipFw2CountRatio);


                // input side data 4
                // int ipRandCountThreshold = 2;

                auto ipRandCountThreshold_d = ctx->GetInputSideData()[4];
                SPDLOG_DEBUG(
                        "Task {}: Consume side data ipRandCountThreshold: {}",
                        name_,
                        ipRandCountThreshold_d);

                auto ipRandCountThreshold_opt =
                        ipRandCountThreshold_d.Consume<int>();
                if (!ipRandCountThreshold_opt)
                {
                        SPDLOG_WARN(
                                "Task {}: Consume() ipRandCountThreshold returns NULL, "
                                "wait for input ...",
                                name_);
                }

                auto ipRandCountThreshold = *(ipRandCountThreshold_opt.value());
                SPDLOG_DEBUG(
                        "Task {}: ipRandCountThreshold: {}", name_, ipRandCountThreshold);


                // input side data 5
                // double_t ipRandCountRatio = 0.5;


                auto ipRandCountRatio_d = ctx->GetInputSideData()[5];
                SPDLOG_DEBUG(
                        "Task {}: Consume side data ipRandCountRatio: {}",
                        name_,
                        ipRandCountRatio_d);

                auto ipRandCountRatio_opt =
                        ipRandCountRatio_d.Consume<double_t>();
                if (!ipRandCountRatio_opt)
                {
                        SPDLOG_WARN(
                                "Task {}: Consume() ipRandCountRatio returns NULL, "
                                "wait for input ...",
                                name_);
                }

                auto ipRandCountRatio = *(ipRandCountRatio_opt.value());
                SPDLOG_DEBUG(
                        "Task {}: ipRandCountRatio: {}", name_, ipRandCountRatio);


                snding::AttackerIPMergeResult nodeRet;

                // merge begin
                // std::unordered_set<uint32_t> all_attack_ips;   // 全部IP
                int ipCountSum = 0;
                for (auto&& ip_count : ipCountDF)
                {
                        ipCountSum += ip_count.second;
                        // all_attack_ips.insert(ip_count.first);
                }

                // Program1, ip_suspect_fw4, process mask=32
                std::vector<uint32_t> ipFw4;
                // masklen is 24~32, as /24, /27 ...
                std::vector<CIDR> attacker_ip_32;
                for (auto&& ip_count : ipCountDF)
                {
                        if (ip_count.second >= ipCountSum * ipFw4CountRatio)
                        {
                                ipFw4.push_back(ip_count.first);
                                attacker_ip_32.emplace_back(ip_count.first, 32);
                        }
                }

                nodeRet.attackerIPs.insert(
                    nodeRet.attackerIPs.end(), attacker_ip_32.begin(),
                    attacker_ip_32.end());

                // erase, rest is ipCountDF1
                for (auto&& ip : ipFw4)
                {
                        ipCountDF.erase(ip);
                }

                if (ipCountDF.size() == 0)
                {
                        // 直接ipFw3和ipCountDF2为空，不需要走这一步和后续的流程了
                        ctx->Outputs()[0].AddDatum(Datum(std::move(nodeRet)));

                        return 0;
                }

                // Program2, ip_suspect_fw3, process 24<=mask<32

                // key: as 1.2.3.0/24, masklen is 24, not others
                // value: all count for ip belongs to the key
                std::unordered_map<uint32_t, int> ipCountFw3DF;

                // key: as 1.2.3.0/24, masklen is 24, not others
                // value: all original ip(/32) belongs to the key from input
                std::unordered_map<uint32_t, std::vector<uint32_t>> origInputIPs;

                // now, ipCountDF is just ipCountDF1
                for (auto&& ip_count : ipCountDF)
                {
                        auto& cnt = ipCountFw3DF[ip_count.first & 0xFFFFFF00];
                        cnt += ip_count.second;

                        auto& vec = origInputIPs[ip_count.first & 0xFFFFFF00];
                        vec.push_back(ip_count.first);
                }

                // masklen is 24~32, as /24, /27 ...
                std::vector<CIDR> attacker_ip_24;
                // FwIPs, masklen is 24
                std::vector<uint32_t> ipSusFw3IPs;
                if (ipCountFw3DF.size() >= ipCountSum * ipFw3CountRatio)
                {
                        for (auto&& ip_count : ipCountFw3DF)
                        {
                                if (ip_count.second >= ipCountSum * ipFw3CountRatio / 2)
                                {
                                        ipSusFw3IPs.push_back(ip_count.first);
                                }
                        }
                }
                else
                {
                        for (auto&& ip_count : ipCountFw3DF)
                        {
                                if (ip_count.second >= ipCountSum * ipFw3CountRatio)
                                {
                                        ipSusFw3IPs.push_back(ip_count.first);
                                }
                        }
                }

                // 遍历每一个 ipSusFw3IP,收集所有的攻击源IP段结果
                for (auto&& ipSusFw3IP : ipSusFw3IPs)
                {
                        std::vector<uint8_t> ipFw3P4List;
                        for (auto&& ip : ipCountDF)
                        {
                                if (belongs_24(ipSusFw3IP, ip.first))
                                {
                                        ipFw3P4List.push_back(ip.first & 0x000000FF);
                                }
                        }

                        if (ipFw3P4List.size() == 0)
                        {
                                continue;
                        }

                        auto minPos =
                            std::min_element(ipFw3P4List.begin(), ipFw3P4List.end());
                        auto ipSusFw3Min = *minPos;

                        auto maxPos =
                            std::max_element(ipFw3P4List.begin(), ipFw3P4List.end());
                        auto ipSusFw3Max = *maxPos;

                        auto ipFw3Range = ipSusFw3Max - ipSusFw3Min;
                        if (ipFw3Range >= 200)
                        {
                                attacker_ip_24.emplace_back(ipSusFw3IP, 24);
                        }
                        else
                        {
                                if (ipFw3P4List.size() >= ipSegCoverThreshold)
                                {
                                        attacker_ip_24.emplace_back(ipSusFw3IP, 24);
                                }
                                else
                                {
                                        dni::CIDR ipFw3_cidrs_min = {
                                            ipSusFw3IP + ipSusFw3Min, 32};
                                        dni::CIDR ipFw3_cidrs_max = {
                                            ipSusFw3IP + ipSusFw3Max, 32};
                                        // 调用netaddr.iprange_to_cidrs()函数，自动生成ipFw3_network_cidrs，可能会有多个输出，mask介于24～32
                                        std::vector<CIDR> cidrs = iprange_to_cidrs(
                                            ipFw3_cidrs_min, ipFw3_cidrs_max);
                                        // 输出识别子掩码（mask>24）的攻击源IP段
                                        attacker_ip_24.insert(
                                            attacker_ip_24.end(), cidrs.begin(),
                                            cidrs.end());
                                }
                        }
                }

                nodeRet.attackerIPs.insert(
                    nodeRet.attackerIPs.end(), attacker_ip_24.begin(),
                    attacker_ip_24.end());

                // erase, rest is ipCountDF2
                for (auto&& ipSusFw3IP : ipSusFw3IPs)
                {
                        auto orig_ips = origInputIPs[ipSusFw3IP];
                        for (auto&& orig_ip : orig_ips)
                        {
                                ipCountDF.erase(orig_ip);
                        }
                }
                if (ipCountDF.size() == 0)
                {
                        // 直接ipFw2，ipCountDF3为空，不需要走这一步和后续的流程了
                        ctx->Outputs()[0].AddDatum(Datum(std::move(nodeRet)));

                        return 0;
                }

                // Program3, ip_suspect_fw2, process 16<=mask<24

                // key: as 1.2.3.0/16, masklen is 16, not others
                // value: all count for ip belongs to the key
                std::unordered_map<uint32_t, int> ipCountFw2DF;
                // key: as 1.2.3.0/16, masklen is 16, not others
                // value: all original ip(/32) belongs to the key from input
                origInputIPs.clear();

                // now, ipCountDF is just ipCountDF2
                for (auto&& ip_count : ipCountDF)
                {
                        auto& cnt = ipCountFw2DF[ip_count.first & 0xFFFF0000];
                        cnt += ip_count.second;

                        auto& vec = origInputIPs[ip_count.first & 0xFFFF0000];
                        vec.push_back(ip_count.first);
                }

                // masklen is 16~24, as /16, /19 ...
                std::vector<CIDR> attacker_ip_16;
                // FwIPs, masklen is 16
                std::vector<uint32_t> ipSusFw2IPs;
                if (ipCountFw2DF.size() >= ipCountSum * ipFw2CountRatio)
                {
                        for (auto&& ip_count : ipCountFw2DF)
                        {
                                if (ip_count.second >= ipCountSum * ipFw2CountRatio / 2)
                                {
                                        ipSusFw2IPs.push_back(ip_count.first);
                                }
                        }
                }
                else
                {
                        for (auto&& ip_count : ipCountFw2DF)
                        {
                                if (ip_count.second >= ipCountSum * ipFw2CountRatio)
                                {
                                        ipSusFw2IPs.push_back(ip_count.first);
                                }
                        }
                }

                // 遍历每一个 ipSusFw2IP,收集所有的攻击源IP段结果
                for (auto&& ipSusFw2IP : ipSusFw2IPs)
                {
                        std::vector<uint8_t> ipFw2P3List;
                        std::vector<uint8_t> ipFw2P4List;
                        for (auto&& ip : ipCountDF)
                        {
                                if (belongs_16(ipSusFw2IP, ip.first))
                                {
                                        ipFw2P3List.push_back(ip.first & 0x0000FF00);
                                        ipFw2P4List.push_back(ip.first & 0x000000FF);
                                }
                        }

                        if (ipFw2P3List.size() == 0 || ipFw2P4List.size() == 0)
                        {
                                continue;
                        }

                        if (ipFw2P3List.size() >= ipSegCoverThreshold &&
                            ipFw2P4List.size() >= ipSegCoverThreshold)
                        {
                                attacker_ip_16.emplace_back(ipSusFw2IP, 16);
                        }
                        else
                        {
                                auto minPos = std::min_element(
                                    ipFw2P3List.begin(), ipFw2P3List.end());
                                auto maxPos = std::max_element(
                                    ipFw2P3List.begin(), ipFw2P3List.end());

                                dni::CIDR ipFw2_cidrs_min = {
                                    ipSusFw2IP + ((*minPos) << 8), 24};
                                dni::CIDR ipFw2_cidrs_max = {
                                    ipSusFw2IP + ((*maxPos) << 8), 24};
                                // 调用netaddr.iprange_to_cidrs()函数，自动生成ipFw2_network_cidrs，可能会有多个输出，mask介于16～24
                                std::vector<CIDR> cidrs =
                                    iprange_to_cidrs(ipFw2_cidrs_min, ipFw2_cidrs_max);
                                // 输出识别子掩码（mask>16）的攻击源IP段
                                attacker_ip_16.insert(
                                    attacker_ip_16.end(), cidrs.begin(), cidrs.end());
                        }
                }

                nodeRet.attackerIPs.insert(
                    nodeRet.attackerIPs.end(), attacker_ip_16.begin(),
                    attacker_ip_16.end());

                // erase, rest is ipCountDF3
                for (auto&& ipSusFw2IP : ipSusFw2IPs)
                {
                        auto orig_ips = origInputIPs[ipSusFw2IP];
                        for (auto&& orig_ip : orig_ips)
                        {
                                ipCountDF.erase(orig_ip);
                        }
                }
                if (ipCountDF.size() == 0)
                {
                        // 直接ipRand，ipCountDF4为空，不需要走这一步和后续的流程了
                        ctx->Outputs()[0].AddDatum(Datum(std::move(nodeRet)));

                        return 0;
                }

                // Program4, ip_suspect_rand
                // 全部IP： 		ipCountDF3 包含的IP
                // 全部IP - 全网已知IP：  所有未知IP
                // 未知正常IP：     ipCountDF3中的统计次数 大于 2
                // 未知随机IP：     所有未知IP - 未知正常IP
                // ipCountDF3 包含的IP
                std::vector<uint32_t> all_DF3_ips;
                for (auto&& ip_count : ipCountDF)
                {
                        all_DF3_ips.push_back(ip_count.first);
                }

                // calc all_DF3_ips - all_known_ips
                std::unordered_set<uint32_t> all_unknown_ips;   // 所有未知IP
                std::sort(all_DF3_ips.begin(), all_DF3_ips.end());
                std::sort(all_known_ips.begin(), all_known_ips.end());
                std::set_difference(
                    all_DF3_ips.begin(), all_DF3_ips.end(), all_known_ips.begin(),
                    all_known_ips.end(),
                    std::inserter(all_unknown_ips, all_unknown_ips.begin()));

                int ipUnknownRandCountSum = 0;
                // std::unordered_set<uint32_t> randIPs;                     //
                // debug
                std::unordered_map<uint32_t, int> ipCountDF4;   // debug
                nodeRet.randomIPs.clear();
                for (auto&& ip : all_unknown_ips)
                {
                        auto cnt = ipCountDF[ip];
                        if (cnt <= ipRandCountThreshold)
                        {
                                ipUnknownRandCountSum += cnt;

                                nodeRet.randomIPs.insert(ip);
                                ipCountDF4[ip] = cnt;
                        }
                }

                bool is_random = false;
                if (ipUnknownRandCountSum >= ipCountSum * ipRandCountRatio)
                {
                        is_random = true;

                        // debug print for randIPs and ipCountDF4
                }
                else
                {
                        nodeRet.randomIPs.clear();
                        ipCountDF4.clear();
                }

                nodeRet.containRandomAttack = is_random;

                ctx->Outputs()[0].AddDatum(Datum(std::move(nodeRet)));

                return 0;
        }

        // 1st arg `cidr` actually is CIDR{ip, 24},
        // 2nd arg `ip` actually is CIDR{ip, 32},
        // `ip`为`cidr`的匹配集的子集等价于:
        // `cidr`的前缀长度不大于32的且`cidr`和`ip`的前`cidr`.len位完全相同
        bool SndAttackerIPMergeTask::belongs_24(uint32_t cidr_24, uint32_t ip)
        {
                return !((cidr_24 ^ ip) >> 8);
        }

        // 1st arg `cidr` actually is CIDR{ip, 16},
        // 2nd arg `ip` actually is CIDR{ip, 32},
        // `ip`为`cidr`的匹配集的子集等价于:
        // `cidr`的前缀长度不大于32的且`cidr`和`ip`的前`cidr`.len位完全相同
        bool SndAttackerIPMergeTask::belongs_16(uint32_t cidr_16, uint32_t ip)
        {
                return !((cidr_16 ^ ip) >> 16);
        }

        REGISTER(SndAttackerIPMergeTask);

}   // namespace dni
