#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "dni/framework/framework.h"
#include "dni/framework/utils/proto.h"
#include "dni/tasks/snding/sip_base_merge_task.pb.h"
#include "snding_defines.h"
#include "spdlog/spdlog.h"

namespace dni {

class SndSIPBaseMergeDeDupTask: public TaskBase {
public:
        SndSIPBaseMergeDeDupTask(): name_("SndSIPBaseMergeDeDupTask") {}
        ~SndSIPBaseMergeDeDupTask() override {}

        int Open(TaskContext* ctx) override;

        int Process(TaskContext* ctx) override;

        int Close(TaskContext* ctx) override
        {
                SPDLOG_DEBUG("{}: closing ...", name_);
                return 0;
        }

private:
        std::string name_;

        std::string calc_number_stat_type(
            const std::unordered_set<uint32_t>& uset,
            double_t num_ratioMin,
            double_t num_ratioMax,
            int num_keyLenThresh,
            const ProtoStrings& num_stat_type);

        std::unordered_map<int, std::string> calc_proto_stat_type(
            const std::unordered_map<int, int>& proto_map,
            int numValueSum,
            double_t proto_ratioMin,
            double_t proto_ratioMax,
            const ProtoStrings& proto_stat_type);

        bool belongs(CIDR cidr, uint32_t ip);

        SndSIPBaseMergeTaskOptions options_;
        double num_ratioMin_;
        double num_ratioMax_;
        int num_keyLenThresh_;
        double proto_ratioMin_;
        double proto_ratioMax_;
};

int SndSIPBaseMergeDeDupTask::Open(TaskContext* ctx)
{
        name_ += "(" + ctx->Name() + ")";
        SPDLOG_DEBUG("{}: open task ...", name_);

        options_ = ctx->Options<SndSIPBaseMergeTaskOptions>();
        num_ratioMin_ = options_.num_stat().ratiomin();
        num_ratioMax_ = options_.num_stat().ratiomax();
        num_keyLenThresh_ = options_.num_stat().keylenthresh();
        if (options_.num_stat().label_size() != 4)
        {
                SPDLOG_ERROR(
                    "{}: invalid num stat labels, expect 4, get {}",
                    name_,
                    options_.num_stat().label_size());
                return -1;
        }
        proto_ratioMin_ = options_.proto_stat().ratiomin();
        proto_ratioMax_ = options_.proto_stat().ratiomax();
        if (options_.proto_stat().label_size() != 3)
        {
                SPDLOG_ERROR(
                    "{}: invalid proto stat labels, expect 3, get {}",
                    name_,
                    options_.proto_stat().label_size());
                return -1;
        }
        SPDLOG_DEBUG(
            "{}: num stat ratioMin: {}, ratioMax: {}, keyLenThresh: {}, labels: {}; "
            "proto stat ratioMin: {}, ratioMax: {}, labels: {}",
            name_, num_ratioMin_, num_ratioMax_, num_keyLenThresh_,
            options_.num_stat().label(), proto_ratioMin_, proto_ratioMax_,
            options_.proto_stat().label());

        return 0;
}

int SndSIPBaseMergeDeDupTask::Process(TaskContext* ctx)
{
        // input0, packets
        Datum packets_d = ctx->Inputs().Tag("PACKET").Value();
        SPDLOG_DEBUG("{}: Consume packets: {}", name_, packets_d);
        auto packets_opt =
            packets_d.Consume<std::vector<std::unordered_map<std::string, uint32_t>>>();
        if (!packets_opt)
        {
                SPDLOG_CRITICAL("{}: invalid input packets", name_);
                return -1;
        }
        auto packets = *(packets_opt.value());
        // SPDLOG_DEBUG("{}: packets: {}", name_, packets);

        // input1, cidr merge result
        Datum attacker_ip_merge_d = ctx->Inputs().Tag("SIP").Value();
        SPDLOG_DEBUG("{}: Consume attacker_ip_merge: {}", name_, attacker_ip_merge_d);
        auto attacker_ip_merge_opt =
            attacker_ip_merge_d.Consume<snding::AttackerIPMergeResult>();
        if (!attacker_ip_merge_opt)
        {
                SPDLOG_CRITICAL("{}: invalid input attacker_ip_merge", name_);
                return -1;
        }
        auto attacker_ip_merge = *(attacker_ip_merge_opt.value());
        SPDLOG_TRACE("{}: attacker_ip_merge: {}", name_, attacker_ip_merge);

        // input 2, host nic name of the packets
        Datum host_nic_name_d = ctx->Inputs().Tag("NIC").Value();
        SPDLOG_DEBUG("{}: Consume host_nic_name: {}", name_, host_nic_name_d);
        auto host_nic_name_opt = host_nic_name_d.Consume<std::string>();
        if (!host_nic_name_opt)
        {
                SPDLOG_CRITICAL("{}: invalid input host_nic_name", name_);
                return -1;
        }
        auto host_nic_name = *(host_nic_name_opt.value());
        SPDLOG_DEBUG("{}: host_nic_name: {}", name_, host_nic_name);

        // sip based merge
        std::vector<std::string> cidr_string_vec;
        std::map<std::string, CIDR> convert_map;
        for (auto&& ip : attacker_ip_merge.attackerIPs)
        {
                cidr_string_vec.emplace_back(
                    std::move(std::to_string(ip.ip) + "/" + std::to_string(ip.len)));

                convert_map[cidr_string_vec.back()] = ip;
        }

        // key will be CIDR, tmp use string now, string is "uint32/len"
        std::unordered_map<std::string, snding::SIPBaseMergeResult> all_merge_ret;

        // sip based merge, step 1, non-random merge sip cidr based merge of all packets
        // packet: std::unordered_map<std::string, uint32_t>

        // ip_positions is for optimize in random ip merge
        // key: ip, value: positions of this ip
        std::unordered_map<uint32_t, std::vector<int>> ip_positions;
        // cidr_include_positions is for optimize in ...
        // key: same as all_merge_ret, string is "uint32/len"
        // value: positions of this ip
        std::unordered_map<std::string, std::vector<int>> cidr_include_positions;
        int index = 0;
        bool is_belong = false;
        for (auto&& packet : packets)
        {
                is_belong = false;
                for (size_t i = 0; i < attacker_ip_merge.attackerIPs.size(); ++i)
                {
                        if (belongs(attacker_ip_merge.attackerIPs[i], packet["SIP"]))
                        {
                                snding::SIPBaseMergeResult& ret =
                                    all_merge_ret[cidr_string_vec[i]];
                                ret.packet_total++;
                                ret.sport.insert(packet["SPort"]);
                                ret.dport.insert(packet["DPort"]);
                                ret.length.insert(packet["Length"]);

                                int& count = ret.protocol[packet["Protocol"]];
                                count++;

                                ret.dstIP.insert(packet["DIP"]);

                                auto& positions =
                                    cidr_include_positions[cidr_string_vec[i]];
                                positions.push_back(index);

                                is_belong = true;

                                break;
                        }
                }

                auto& pos_vec = ip_positions[packet["SIP"]];
                pos_vec.push_back(index);

                if (!is_belong)
                {
                        auto& positions = cidr_include_positions["RANDOM"];
                        positions.push_back(index);
                }

                index++;
        }

        // sip based merge, step 2, random merge
        if (attacker_ip_merge.containRandomAttack)
        {
                SPDLOG_DEBUG("{}: with random IPs", name_);

                snding::SIPBaseMergeResult& randomMerge = all_merge_ret["RANDOM"];

                for (auto&& random_ip : attacker_ip_merge.randomIPs)
                {
                        for (auto&& index : ip_positions[random_ip])
                        {
                                auto p = packets[index];

                                randomMerge.packet_total++;
                                randomMerge.sport.insert(p["SPort"]);
                                randomMerge.dport.insert(p["DPort"]);
                                randomMerge.length.insert(p["Length"]);

                                int& count = randomMerge.protocol[p["Protocol"]];
                                count++;

                                randomMerge.dstIP.insert(p["DIP"]);
                        }
                }
        }

        // key will be CIDR, tmp use string now, string is "uint32/len".
        // if contain random, the key is "RANDOM".
        // analyze the situation of sport/dport/length/protocol.
        // no need to calc sip stat, because from upstream node,
        // all packet ip split two part:
        // (a) cidr merge ip/32, or ip/(<32), it is abs CENTRALIZED
        // (b) random ip, it is abs RANDOM, so not calc
        std::unordered_map<std::string, snding::SIPBaseMergeStats> all_stat;
        for (auto&& merge_ret : all_merge_ret)
        {
                // process number
                snding::SIPBaseMergeStats& stat = all_stat[merge_ret.first];
                stat.hostNicSign = host_nic_name;

                auto numValueSum = merge_ret.second.packet_total;

                auto sport_stat_type = calc_number_stat_type(
                    merge_ret.second.sport,
                    num_ratioMin_,
                    num_ratioMax_,
                    num_keyLenThresh_,
                    options_.num_stat().label());

                auto dport_stat_type = calc_number_stat_type(
                    merge_ret.second.dport,
                    num_ratioMin_,
                    num_ratioMax_,
                    num_keyLenThresh_,
                    options_.num_stat().label());

                auto length_stat_type = calc_number_stat_type(
                    merge_ret.second.length,
                    num_ratioMin_,
                    num_ratioMax_,
                    num_keyLenThresh_,
                    options_.num_stat().label());

                // for sport, dport, length, if the stat_type is CENTRALIZED, use
                // merge_ret.second.sport/dport/length as the merge result for next step;
                // otherwise, use the stat_type name shown in the next step.
                stat.srcPort.stat_type = sport_stat_type;
                if (sport_stat_type == options_.num_stat().label()[0])
                {
                        stat.srcPort.value = std::move(merge_ret.second.sport);
                }
                stat.dstPort.stat_type = dport_stat_type;
                if (dport_stat_type == options_.num_stat().label()[0])
                {
                        stat.dstPort.value = std::move(merge_ret.second.dport);
                }
                stat.length.stat_type = length_stat_type;
                if (length_stat_type == options_.num_stat().label()[0])
                {
                        stat.length.value = std::move(merge_ret.second.length);
                }

                // process protocol
                auto prot = calc_proto_stat_type(
                    merge_ret.second.protocol,
                    numValueSum,
                    proto_ratioMin_,
                    proto_ratioMax_,
                    options_.proto_stat().label());
                stat.protocol.stat_types = std::move(prot);

                if (merge_ret.first == "RANDOM")
                {
                        stat.isSrcIPRandom = true;
                        // downstream node will use "RANDOM-dstip" to match attack link.
                        stat.dstIP.value = std::move(merge_ret.second.dstIP);
                }
                else
                {
                        // downstream node will use "srcip-dstip" to match attack link.
                        stat.srcIP = convert_map[merge_ret.first];
                        // stat.dstIP = {packets.at(0)["DIP"], 32};
                        stat.dstIP.value = std::move(merge_ret.second.dstIP);
                }
        }

        std::string rule_elements = "";

        // std::unordered_map<std::string, std::unordered_set<int>> length_set;
        std::unordered_map<std::string, std::unordered_map<std::string, std::string>>
            rules;
        for (auto&& stat : all_stat)
        {
                // use stat to replace each packet in packets, the packet ip belongs to
                // the stat.first, or it is RANDOM ip

                auto& positions = cidr_include_positions[stat.first];

                for (auto&& index : positions)
                {
                        rule_elements = "";
                        std::unordered_map<std::string, std::string> rule_stat;

                        // this is original packet
                        auto& p = packets[index];
                        rule_elements += (stat.first + "--");
                        rule_stat["SIP"] = stat.first;

                        rule_elements += (std::to_string(p["DIP"]) + "--");
                        rule_stat["DIP"] = std::to_string(p["DIP"]);

                        if (stat.second.srcPort.stat_type == "centralize")
                        {
                                rule_elements += (std::to_string(p["SPort"]) + "--");
                                rule_stat["SPort"] = std::to_string(p["SPort"]);
                        }
                        else
                        {
                                rule_elements += (stat.second.srcPort.stat_type + "--");
                                rule_stat["SPort"] = stat.second.srcPort.stat_type;
                        }

                        if (stat.second.dstPort.stat_type == "centralize")
                        {
                                rule_elements += (std::to_string(p["DPort"]) + "--");
                                rule_stat["DPort"] = std::to_string(p["DPort"]);
                        }
                        else
                        {
                                rule_elements += (stat.second.dstPort.stat_type + "--");
                                rule_stat["DPort"] = stat.second.dstPort.stat_type;
                        }

                        rule_elements += (std::to_string(p["Protocol"]) + "--");
                        rule_stat["Protocol"] = std::to_string(p["Protocol"]);

                        rule_stat["hostNicSign"] = stat.second.hostNicSign;

                        // collect length if need in stat.second.length.stat_type,
                        // stat.second.length.value
                        // length join in rule_elements
                        if (stat.second.length.stat_type == "centralize")
                        {
                                rule_elements += std::to_string(p["Length"]);
                                rule_stat["Length"] = std::to_string(p["Length"]);
                        }
                        else
                        {
                                rule_elements += stat.second.length.stat_type;
                                rule_stat["Length"] = stat.second.length.stat_type;
                        }

                        // add to ...
                        rules[rule_elements] = rule_stat;
                }
        }

        SPDLOG_DEBUG("{}: after calculation, all_stat: {}", name_, all_stat);
        SPDLOG_DEBUG("{}: after calculation, rules: {}", name_, rules);

        ctx->Outputs().Get("CIDR", 0).AddDatum(Datum(std::move(all_stat)));
        ctx->Outputs().Get("CIDR", 1).AddDatum(Datum(std::move(rules)));

        return 0;
}

// 2nd arg `ip` actually is CIDR{ip, 32}
// `ip`为`cidr`的匹配集的子集等价于:
// `cidr`的前缀长度不大于32的且`cidr`和`ip`的前`cidr`.len位完全相同
bool SndSIPBaseMergeDeDupTask::belongs(CIDR cidr, uint32_t ip)
{
        if (cidr.len > 32)
                return false;
        if ((cidr.ip ^ ip) >> (32 - cidr.len))
                return false;
        return true;
}

std::string SndSIPBaseMergeDeDupTask::calc_number_stat_type(
    const std::unordered_set<uint32_t>& uset,
    double_t num_ratioMin,
    double_t num_ratioMax,
    int num_keyLenThresh,
    const ProtoStrings& num_stat_type)
{
        // numKeyLen
        auto numKeyLen = uset.size();
        SPDLOG_DEBUG("{}: numKeyLen: {}", name_, numKeyLen);

        // keySeriesDiffTypeNum
        std::vector<uint32_t> keys;
        for (const auto& key : uset)
        {
                keys.push_back(key);
        }
        std::sort(keys.begin(), keys.end());
        std::unordered_set<uint32_t> keyDiffs;
        for (size_t i = 0; i < keys.size() - 1; i++)
        {
                keyDiffs.insert(keys[i + 1] - keys[i]);
                // SPDLOG_DEBUG(
                //     "{}: keyDiffs.insert: {}", name_, keys[i + 1]
                //     - keys[i]);
        }
        auto keySeriesDiffTypeNum = keyDiffs.size();

        SPDLOG_DEBUG("{}: keySeriesDiffTypeNum: {}", name_, keySeriesDiffTypeNum);

        std::string stat_type;
        double_t min_ = (double_t) numKeyLen * num_ratioMin;
        double_t max_ = (double_t) numKeyLen * num_ratioMax;
        if (numKeyLen < num_keyLenThresh)
        {
                stat_type = num_stat_type[0];
        }
        else
        {
                if ((double_t) keySeriesDiffTypeNum <= min_)
                {
                        stat_type = num_stat_type[1];
                }
                else if ((double_t) keySeriesDiffTypeNum >= max_)
                {
                        stat_type = num_stat_type[2];
                }
                else
                {
                        stat_type = num_stat_type[3];
                }
        }

        SPDLOG_DEBUG("{}: num stat type: {}", name_, stat_type);

        return stat_type;
}

std::unordered_map<int, std::string> SndSIPBaseMergeDeDupTask::calc_proto_stat_type(
    const std::unordered_map<int, int>& proto_map,
    int numValueSum,
    double_t proto_ratioMin,
    double_t proto_ratioMax,
    const ProtoStrings& proto_stat_type)
{
        SPDLOG_DEBUG("{}: numValueSum: {}", name_, numValueSum);
        std::unordered_map<int, std::string> stats;
        double_t min_ = (double_t) numValueSum * proto_ratioMin;
        double_t max_ = (double_t) numValueSum * proto_ratioMax;

        for (auto&& proto : proto_map)
        {
                auto protoCountValue = proto.second;
                std::string stat_type = proto_stat_type[2];
                if (protoCountValue >= max_)
                {
                        stat_type = proto_stat_type[0];
                }
                else
                {
                        if (protoCountValue >= min_)
                        {
                                stat_type = proto_stat_type[1];
                        }
                }

                SPDLOG_DEBUG(
                    "{}: protocol: {}, protoCountValue: {}, proto stat type: {}",
                    name_,
                    proto.first,
                    protoCountValue,
                    stat_type);

                stats[proto.first] = stat_type;
        }

        return std::move(stats);
}

REGISTER(SndSIPBaseMergeDeDupTask);

}   // namespace dni
