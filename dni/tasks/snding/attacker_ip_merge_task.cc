#include <algorithm>
#include <unordered_map>
#include <vector>

#include "dni/framework/formats/cidr.h"
#include "dni/framework/framework.h"
#include "dni/tasks/snding/attacker_ip_merge_task.pb.h"
#include "snding_defines.h"
#include "spdlog/spdlog.h"

namespace dni {

class SndAttackerIPMergeTask: public TaskBase {
public:
        SndAttackerIPMergeTask(): name_("SndAttackerIPMergeTask") {}
        ~SndAttackerIPMergeTask() override {}

        int Open(TaskContext* ctx) override;

        int Process(TaskContext* ctx) override;

        int Close(TaskContext* ctx) override
        {
                SPDLOG_DEBUG("{}: closing ...", name_);

                return 0;
        }

private:
        // 1st arg `cidr` actually is CIDR{ip, 24},
        // 2nd arg `ip` actually is CIDR{ip, 32},
        // `ip`为`cidr`的匹配集的子集等价于:
        // `cidr`的前缀长度不大于32的且`cidr`和`ip`的前`cidr`.len位完全相同
        bool belongs_24(uint32_t cidr_24, uint32_t ip) { return !((cidr_24 ^ ip) >> 8); }

        // 1st arg `cidr` actually is CIDR{ip, 16},
        // 2nd arg `ip` actually is CIDR{ip, 32},
        // `ip`为`cidr`的匹配集的子集等价于:
        // `cidr`的前缀长度不大于32的且`cidr`和`ip`的前`cidr`.len位完全相同
        bool belongs_16(uint32_t cidr_16, uint32_t ip) { return !((cidr_16 ^ ip) >> 16); }

        std::string name_;

        SndAttackerIPMergeTaskOptions options_;
        double_t ipFw4CountRatio_ = 0.1;
        double_t ipFw3CountRatio_ = 0.2;
        double_t ipSegCoverThreshold_ = 100;
        double_t ipFw2CountRatio_ = 0.4;
        int ipRandCountThreshold_ = 2;
        double_t ipRandCountRatio_ = 0.5;
};

int SndAttackerIPMergeTask::Open(TaskContext* ctx)
{
        name_ += "(" + ctx->Name() + ")";
        SPDLOG_DEBUG("{}: open task ...", name_);

        options_ = ctx->Options<SndAttackerIPMergeTaskOptions>();
        if (options_.has_ipfw4countratio())
                ipFw4CountRatio_ = options_.ipfw4countratio();
        if (options_.has_ipfw3countratio())
                ipFw3CountRatio_ = options_.ipfw3countratio();
        if (options_.has_ipsegcoverthreshold())
                ipSegCoverThreshold_ = options_.ipsegcoverthreshold();
        if (options_.has_ipfw2countratio())
                ipFw2CountRatio_ = options_.ipfw2countratio();
        if (options_.has_iprandcountthreshold())
                ipRandCountThreshold_ = options_.iprandcountthreshold();
        if (options_.has_iprandcountratio())
                ipRandCountRatio_ = options_.iprandcountratio();

        SPDLOG_DEBUG(
            "ipFw4CountRatio_:{},ipFw3CountRatio_:{},ipSegCoverThreshold_:{},"
            "ipFw2CountRatio_:{},ipRandCountThreshold_:{},ipRandCountRatio_:{}",
            ipFw4CountRatio_, ipFw3CountRatio_, ipSegCoverThreshold_, ipFw2CountRatio_,
            ipRandCountThreshold_, ipRandCountRatio_);

        return 0;
}

int SndAttackerIPMergeTask::Process(TaskContext* ctx)
{
        // input 0, key: srcip/32, value: count
        Datum ip_map_d = ctx->Inputs().Tag("SIP").Value();
        SPDLOG_DEBUG("{}: Consume ip_map: {}", name_, ip_map_d);
        auto ip_map_opt = ip_map_d.Consume<std::unordered_map<uint32_t, int>*>();
        if (!ip_map_opt)
        {
                SPDLOG_CRITICAL("{}: invalid input ip_map", name_);
                return -1;
        }
        auto ipCountDF = *(ip_map_opt.value());
        auto ipCountDFSize = ipCountDF->size();
        SPDLOG_DEBUG("{}: ipCountDF size: {}", name_, ipCountDFSize);
        SPDLOG_TRACE("{}: ipCountDF: {}", name_, ipCountDF);

        // input 1, all known IPs in net, /32
        Datum all_known_ips_d = ctx->Inputs().Tag("KNOWN_IPS").Value();
        SPDLOG_DEBUG("{}: Consume all_known_ips: {}", name_, all_known_ips_d);
        auto all_known_ips_opt = all_known_ips_d.Consume<std::vector<uint32_t>>();
        if (!all_known_ips_opt)
        {
                SPDLOG_CRITICAL("{}: invalid input all_known_ips", name_);
                return -1;
        }
        auto all_known_ips = *(all_known_ips_opt.value());
        SPDLOG_DEBUG("{}: val: {}", name_, all_known_ips);

        snding::AttackerIPMergeResult nodeRet;
        nodeRet.attackerIPs.reserve(10000);
        nodeRet.randomIPs.reserve(10000);

        // merge begin
        // std::unordered_set<uint32_t> all_attack_ips;   // 全部IP
        int ipCountSum = 0;
        for (auto iter = ipCountDF->begin(); iter != ipCountDF->end(); iter++)
        {
                ipCountSum += (iter->second);
        }

        // Program1, ip_suspect_fw4, process mask=32
        std::vector<uint32_t> ipFw4;
        ipFw4.reserve(ipCountDFSize);
        // masklen is 24~32, as /24, /27 ...
        std::vector<CIDR> attacker_ip_32;
        attacker_ip_32.reserve(ipCountDFSize);
        int ipCountSum_ipFw4CountRatio = (int) (ipCountSum * ipFw4CountRatio_);
        for (auto iter1 = ipCountDF->begin(); iter1 != ipCountDF->end(); iter1++)
        {
                if (iter1->second >= ipCountSum_ipFw4CountRatio)
                {
                        ipFw4.push_back(iter1->first);
                        attacker_ip_32.emplace_back(iter1->first, 32);

                        SPDLOG_DEBUG(
                            "{}: will delete after Prog1: {:x}", name_, iter1->first);
                }
        }

        nodeRet.attackerIPs.insert(
            nodeRet.attackerIPs.end(), attacker_ip_32.begin(), attacker_ip_32.end());

        SPDLOG_DEBUG(
            "{}: after Program1, nodeRet.attackerIPs, size: {}\n, {}", name_,
            nodeRet.attackerIPs.size(), nodeRet.attackerIPs);

        // erase, rest is ipCountDF1
        for (auto&& ip : ipFw4)
        {
                ipCountDF->erase(ip);
        }

        if (ipCountDF->size() == 0)
        {
                // 直接ipFw3和ipCountDF2为空，不需要走这一步和后续的流程了
                SPDLOG_DEBUG("{}: after Prog1, no ip left", name_);
                ctx->Outputs()[0].AddDatum(Datum(std::move(nodeRet)));

                return 0;
        }

        SPDLOG_DEBUG("{}: after Prog1, ipCountDF size: {}", name_, ipCountDF->size());

        // Program2, ip_suspect_fw3, process 24<=mask<32

        // key: as 1.2.3.0/24, masklen is 24, not others
        // value: all count for ip belongs to the key
        std::unordered_map<uint32_t, int> ipCountFw3DF;

        // key: as 1.2.3.0/24, masklen is 24, not others
        // value: all original ip(/32) belongs to the key from input
        std::unordered_map<uint32_t, std::vector<uint32_t>> origInputIPs;

        // now, ipCountDF is just ipCountDF1
        for (auto iter2 = ipCountDF->begin(); iter2 != ipCountDF->end(); iter2++)
        {
                auto& cnt = ipCountFw3DF[iter2->first & 0xFFFFFF00];
                cnt += iter2->second;

                auto& vec = origInputIPs[iter2->first & 0xFFFFFF00];
                vec.push_back(iter2->first);
        }

        // masklen is 24~32, as /24, /27 ...
        std::vector<CIDR> attacker_ip_24;
        attacker_ip_24.reserve(10000);
        // FwIPs, masklen is 24
        std::vector<uint32_t> ipSusFw3IPs;
        ipSusFw3IPs.reserve(ipCountFw3DF.size());

        int ipCountSum_ipFw3CountRatio = (int) (ipCountSum * ipFw3CountRatio_);
        int ipCountSum_ipFw3CountRatio_half = (int) (ipCountSum * ipFw3CountRatio_ / 2);
        if (ipCountFw3DF.size() >= ipCountSum_ipFw3CountRatio)
        {
                for (auto&& ip_count : ipCountFw3DF)
                {
                        if (ip_count.second >= ipCountSum_ipFw3CountRatio_half)
                        {
                                SPDLOG_DEBUG(
                                    "{}: >= ipCountSum * ipFw3CountRatio_ / 2, {:x}",
                                    name_, ip_count.first);
                                ipSusFw3IPs.push_back(ip_count.first);
                        }
                }
        }
        else
        {
                for (auto&& ip_count : ipCountFw3DF)
                {
                        if (ip_count.second >= ipCountSum_ipFw3CountRatio)
                        {
                                SPDLOG_DEBUG(
                                    "{}: >= ipCountSum * ipFw3CountRatio_, {:x}", name_,
                                    ip_count.first);
                                ipSusFw3IPs.push_back(ip_count.first);
                        }
                }
        }

        std::vector<uint8_t> ipFw3P4List;
        ipFw3P4List.reserve(ipCountDF->size());
        // 遍历每一个 ipSusFw3IP,收集所有的攻击源IP段结果
        std::vector<uint8_t>::iterator minPos;
        uint8_t ipSusFw3Min;
        std::vector<uint8_t>::iterator maxPos;
        uint8_t ipSusFw3Max;
        int ipFw3Range;
        dni::CIDR ipFw3_cidrs_min = {0, 32};
        dni::CIDR ipFw3_cidrs_max = {0, 32};
        for (auto&& ipSusFw3IP : ipSusFw3IPs)
        {
                SPDLOG_DEBUG("{}: ipSusFw3IP begin ...., {:x}", name_, ipSusFw3IP);

                ipFw3P4List.clear();

                for (auto iter2_2 = ipCountDF->begin(); iter2_2 != ipCountDF->end();
                     iter2_2++)
                {
                        if (belongs_24(ipSusFw3IP, iter2_2->first))
                        {
                                ipFw3P4List.push_back(iter2_2->first & 0x000000FF);
                        }
                }

                if (ipFw3P4List.size() == 0)
                {
                        SPDLOG_DEBUG(
                            "{}: ipSusFw3IP end, no ip ...., {:x}", name_, ipSusFw3IP);
                        continue;
                }

                minPos = std::min_element(ipFw3P4List.begin(), ipFw3P4List.end());
                ipSusFw3Min = *minPos;

                maxPos = std::max_element(ipFw3P4List.begin(), ipFw3P4List.end());
                ipSusFw3Max = *maxPos;

                ipFw3Range = ipSusFw3Max - ipSusFw3Min;
                SPDLOG_DEBUG(
                    "{}: ipSusFw3IP ...., {:x}, {}, {}, {}", name_, ipSusFw3IP,
                    ipSusFw3Max, ipSusFw3Min, ipFw3Range);
                if (ipFw3Range >= 200)
                {
                        SPDLOG_DEBUG(
                            "{}: ipSusFw3IP range >= 200, {:x}", name_, ipSusFw3IP);
                        attacker_ip_24.emplace_back(ipSusFw3IP, 24);
                }
                else
                {
                        if (ipFw3P4List.size() >= ipSegCoverThreshold_)
                        {
                                SPDLOG_DEBUG(
                                    "{}: ipSusFw3IP ipFw3P4List.size: {}", name_,
                                    ipFw3P4List.size());
                                attacker_ip_24.emplace_back(ipSusFw3IP, 24);
                        }
                        else
                        {
                                ipFw3_cidrs_min.ip = ipSusFw3IP + ipSusFw3Min;
                                ipFw3_cidrs_max.ip = ipSusFw3IP + ipSusFw3Max;
                                SPDLOG_DEBUG(
                                    "{}: ipSusFw3IP ...., {:x}, ipFw3_cidrs_min: "
                                    "{:x}/32, ipFw3_cidrs_max: {:x}/32",
                                    name_, ipSusFw3IP, ipFw3_cidrs_min.ip,
                                    ipFw3_cidrs_max.ip);
                                // 调用netaddr.iprange_to_cidrs()函数，自动生成ipFw3_network_cidrs，可能会有多个输出，mask介于24～32
                                std::vector<CIDR> cidrs =
                                    iprange_to_cidrs(ipFw3_cidrs_min, ipFw3_cidrs_max);
                                // 输出识别子掩码（mask>24）的攻击源IP段
                                SPDLOG_DEBUG(
                                    "{}: ipSusFw3IP iprange_to_cidrs, {}", name_, cidrs);
                                attacker_ip_24.insert(
                                    attacker_ip_24.end(), cidrs.begin(), cidrs.end());
                        }
                }

                SPDLOG_DEBUG("{}: ipSusFw3IP end ...., {:x}", name_, ipSusFw3IP);
        }

        SPDLOG_DEBUG(
            "{}: ipSusFw3IP attacker_ip_24, size: {}\n, {}", name_, attacker_ip_24.size(),
            attacker_ip_24);

        nodeRet.attackerIPs.insert(
            nodeRet.attackerIPs.end(), attacker_ip_24.begin(), attacker_ip_24.end());

        SPDLOG_DEBUG(
            "{}: after Program2, nodeRet.attackerIPs, size: {}\n, {}", name_,
            nodeRet.attackerIPs.size(), nodeRet.attackerIPs);

        // erase, rest is ipCountDF2
        for (auto&& ipSusFw3IP : ipSusFw3IPs)
        {
                auto orig_ips = origInputIPs[ipSusFw3IP];
                for (auto&& orig_ip : orig_ips)
                {
                        ipCountDF->erase(orig_ip);
                }
        }
        if (ipCountDF->size() == 0)
        {
                // 直接ipFw2，ipCountDF3为空，不需要走这一步和后续的流程了
                SPDLOG_DEBUG("{}: after Prog2, no ip left", name_);
                ctx->Outputs()[0].AddDatum(Datum(std::move(nodeRet)));

                return 0;
        }

        SPDLOG_DEBUG("{}: after Prog2, ipCountDF size: {}", name_, ipCountDF->size());

        // Program3, ip_suspect_fw2, process 16<=mask<24

        // key: as 1.2.3.0/16, masklen is 16, not others
        // value: all count for ip belongs to the key
        std::unordered_map<uint32_t, int> ipCountFw2DF;
        // key: as 1.2.3.0/16, masklen is 16, not others
        // value: all original ip(/32) belongs to the key from input
        origInputIPs.clear();

        // now, ipCountDF is just ipCountDF2
        for (auto iter3 = ipCountDF->begin(); iter3 != ipCountDF->end(); iter3++)
        {
                auto& cnt = ipCountFw2DF[iter3->first & 0xFFFF0000];
                cnt += iter3->second;

                auto& vec = origInputIPs[iter3->first & 0xFFFF0000];
                vec.push_back(iter3->first);
        }

        // masklen is 16~24, as /16, /19 ...
        std::vector<CIDR> attacker_ip_16;
        attacker_ip_16.reserve(10000);
        // FwIPs, masklen is 16
        std::vector<uint32_t> ipSusFw2IPs;
        ipSusFw2IPs.reserve(ipCountFw2DF.size());

        int ipCountSum_ipFw2CountRatio = (int) (ipCountSum * ipFw2CountRatio_);
        int ipCountSum_ipFw2CountRatio_half = (int) (ipCountSum * ipFw2CountRatio_ / 2);
        if (ipCountFw2DF.size() >= ipCountSum_ipFw2CountRatio)
        {
                for (auto&& ip_count : ipCountFw2DF)
                {
                        if (ip_count.second >= ipCountSum_ipFw2CountRatio_half)
                        {
                                SPDLOG_DEBUG(
                                    "{}: >= ipCountSum * ipFw2CountRatio_ / 2, {:x}",
                                    name_, ip_count.first);
                                ipSusFw2IPs.push_back(ip_count.first);
                        }
                }
        }
        else
        {
                for (auto&& ip_count : ipCountFw2DF)
                {
                        if (ip_count.second >= ipCountSum_ipFw2CountRatio)
                        {
                                SPDLOG_DEBUG(
                                    "{}: >= ipCountSum * ipFw2CountRatio_, {:x}", name_,
                                    ip_count.first);
                                ipSusFw2IPs.push_back(ip_count.first);
                        }
                }
        }

        std::vector<uint8_t> ipFw2P3List;
        ipFw2P3List.reserve(ipCountDF->size());
        std::vector<uint8_t> ipFw2P4List;
        ipFw2P4List.reserve(ipCountDF->size());

        dni::CIDR ipFw2_cidrs_min = {0, 24};
        dni::CIDR ipFw2_cidrs_max = {0, 24};

        // 遍历每一个 ipSusFw2IP,收集所有的攻击源IP段结果
        for (auto&& ipSusFw2IP : ipSusFw2IPs)
        {
                SPDLOG_DEBUG("{}: ipSusFw2IP begin ...., {:x}", name_, ipSusFw2IP);

                ipFw2P3List.clear();
                ipFw2P4List.clear();

                for (auto iter3_2 = ipCountDF->begin(); iter3_2 != ipCountDF->end();
                     iter3_2++)
                {
                        if (belongs_16(ipSusFw2IP, iter3_2->first))
                        {
                                ipFw2P3List.push_back(iter3_2->first & 0x0000FF00);
                                ipFw2P4List.push_back(iter3_2->first & 0x000000FF);
                        }
                }

                if (ipFw2P3List.size() == 0 || ipFw2P4List.size() == 0)
                {
                        SPDLOG_DEBUG(
                            "{}: ipSusFw2IP end, no ip ...., {:x}", name_, ipSusFw2IP);
                        continue;
                }

                SPDLOG_DEBUG(
                    "{}: ipSusFw2IP {:x}, ipFw2P3List.size(): {}, ipFw2P4List.size(): {}",
                    name_, ipSusFw2IP, ipFw2P3List.size(), ipFw2P4List.size());

                if (ipFw2P3List.size() >= ipSegCoverThreshold_ &&
                    ipFw2P4List.size() >= ipSegCoverThreshold_)
                {
                        SPDLOG_DEBUG(
                            "{}: ipSusFw2IP p3 p4 range >= , {:x}", name_, ipSusFw2IP);
                        attacker_ip_16.emplace_back(ipSusFw2IP, 16);
                }
                else
                {
                        minPos = std::min_element(ipFw2P3List.begin(), ipFw2P3List.end());
                        maxPos = std::max_element(ipFw2P3List.begin(), ipFw2P3List.end());

                        ipFw2_cidrs_min.ip = (ipSusFw2IP + ((*minPos) << 8));
                        ipFw2_cidrs_max.ip = (ipSusFw2IP + ((*maxPos) << 8));
                        SPDLOG_DEBUG(
                            "{}: ipSusFw2IP ...., {:x}, ipFw2_cidrs_min: {:x}/32, "
                            "ipFw2_cidrs_max: {:x}/32",
                            name_, ipSusFw2IP, ipFw2_cidrs_min.ip, ipFw2_cidrs_max.ip);
                        // 调用netaddr.iprange_to_cidrs()函数，自动生成ipFw2_network_cidrs，可能会有多个输出，mask介于16～24
                        std::vector<CIDR> cidrs =
                            iprange_to_cidrs(ipFw2_cidrs_min, ipFw2_cidrs_max);
                        // 输出识别子掩码（mask>16）的攻击源IP段
                        SPDLOG_DEBUG("{}: ipSusFw2IP iprange_to_cidrs, {}", name_, cidrs);
                        attacker_ip_16.insert(
                            attacker_ip_16.end(), cidrs.begin(), cidrs.end());
                }

                SPDLOG_DEBUG("{}: ipSusFw2IP end ...., {:x}", name_, ipSusFw2IP);
        }

        SPDLOG_DEBUG(
            "{}: ipSusFw2IP attacker_ip_16, size: {}\n, {}", name_, attacker_ip_16.size(),
            attacker_ip_16);

        nodeRet.attackerIPs.insert(
            nodeRet.attackerIPs.end(), attacker_ip_16.begin(), attacker_ip_16.end());

        SPDLOG_DEBUG(
            "{}: after Program3, nodeRet.attackerIPs, size: {}\n, {}", name_,
            nodeRet.attackerIPs.size(), nodeRet.attackerIPs);

        // erase, rest is ipCountDF3
        for (auto&& ipSusFw2IP : ipSusFw2IPs)
        {
                auto& orig_ips = origInputIPs[ipSusFw2IP];
                for (auto&& orig_ip : orig_ips)
                {
                        ipCountDF->erase(orig_ip);
                }
        }
        if (ipCountDF->size() == 0)
        {
                // 直接ipRand，ipCountDF4为空，不需要走这一步和后续的流程了
                SPDLOG_DEBUG("{}: after Prog3, no ip left", name_);
                ctx->Outputs()[0].AddDatum(Datum(std::move(nodeRet)));

                return 0;
        }

        SPDLOG_DEBUG("{}: after Prog3, ipCountDF size: {}", name_, ipCountDF->size());

        // Program4, ip_suspect_rand
        // 全部IP： 		ipCountDF3 包含的IP
        // 全部IP - 全网已知IP：  所有未知IP
        // 未知正常IP：     ipCountDF3中的统计次数 大于 2
        // 未知随机IP：     所有未知IP - 未知正常IP
        // ipCountDF3 包含的IP
        std::vector<uint32_t> all_DF3_ips;
        all_DF3_ips.reserve(ipCountDF->size());

        for (auto iter4 = ipCountDF->begin(); iter4 != ipCountDF->end(); iter4++)
        {
                all_DF3_ips.push_back(iter4->first);
        }

        // calc all_DF3_ips - all_known_ips
        std::unordered_set<uint32_t> all_unknown_ips;   // 所有未知IP
        all_unknown_ips.reserve(10000);
        std::sort(all_DF3_ips.begin(), all_DF3_ips.end());
        // std::sort(all_known_ips.begin(), all_known_ips.end());
        std::set_difference(
            all_DF3_ips.begin(), all_DF3_ips.end(), all_known_ips.begin(),
            all_known_ips.end(), std::inserter(all_unknown_ips, all_unknown_ips.begin()));

        SPDLOG_DEBUG("{}: all_unknown_ips size: {}", name_, all_unknown_ips.size());

        int ipUnknownRandCountSum = 0;
        nodeRet.randomIPs.clear();
        for (auto&& ip : all_unknown_ips)
        {
                auto cnt = ipCountDF->operator[](ip);
                if (cnt <= ipRandCountThreshold_)
                {
                        ipUnknownRandCountSum += cnt;

                        nodeRet.randomIPs.insert(ip);
                }
        }

        SPDLOG_DEBUG(
            "{}: ipUnknownRandCountSum: {}, nodeRet.randomIPs size: {}, ipCountSum: {}",
            name_, ipUnknownRandCountSum, nodeRet.randomIPs.size(), ipCountSum);

        bool is_random = false;
        if (ipUnknownRandCountSum >= ipCountSum * ipRandCountRatio_)
        {
                is_random = true;
        }
        else
        {
                nodeRet.randomIPs.clear();
        }

        nodeRet.containRandomAttack = is_random;

        SPDLOG_DEBUG("{}: after Prog4, nodeRet: {}", name_, nodeRet);

        ctx->Outputs()[0].AddDatum(Datum(std::move(nodeRet)));

        return 0;
}

REGISTER(SndAttackerIPMergeTask);

}   // namespace dni
