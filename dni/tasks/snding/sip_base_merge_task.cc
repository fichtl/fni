#include <string>
#include <unordered_map>
#include <unordered_set>

#include "dni/framework/framework.h"
#include "snding_defines.h"
#include "spdlog/spdlog.h"

namespace dni {

        class SndSIPBaseMergeTask: public TaskBase {
        public:
                SndSIPBaseMergeTask(): name_("SndSIPBaseMergeTask") {}
                ~SndSIPBaseMergeTask() override {}

                int Open(TaskContext* ctx) override
                {
                        name_ += " " + ctx->Name();
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
                std::string name_;

                std::string calc_number_stat_type(
                    const std::unordered_set<uint32_t>& uset,
                    int numValueSum,
                    double_t num_ratioMin,
                    double_t num_ratioMax,
                    const std::vector<std::string>& num_stat_type);

                std::unordered_map<int, std::string> calc_proto_stat_type(
                    const std::unordered_map<int, int>& proto_map,
                    int numValueSum,
                    double_t proto_ratioMin,
                    double_t proto_ratioMax,
                    const std::vector<std::string>& proto_stat_type);

                bool belongs(CIDR cidr, uint32_t ip);
        };

        int SndSIPBaseMergeTask::Process(TaskContext* ctx)
        {
                // input0, packets
                Datum packets_d = ctx->Inputs()[0].Value();
                SPDLOG_DEBUG("Task {}: Consume packets: {}", name_, packets_d);
                auto packets_opt = packets_d.Consume<
                    std::vector<std::unordered_map<std::string, uint32_t>>>();
                if (!packets_opt)
                {
                        SPDLOG_WARN(
                            "Task {}: Consume() packets returns NULL, wait for "
                            "input ...",
                            name_);
                }
                auto packets = *(packets_opt.value());
                // SPDLOG_DEBUG("Task {}: packets: {}", name_, packets);

                // input1, cidr merge result
                Datum attacker_ip_merge_d = ctx->Inputs()[1].Value();
                SPDLOG_DEBUG(
                    "Task {}: Consume attacker_ip_merge: {}", name_, attacker_ip_merge_d);
                auto attacker_ip_merge_opt =
                    attacker_ip_merge_d.Consume<snding::AttackerIPMergeResult>();
                if (!attacker_ip_merge_opt)
                {
                        SPDLOG_WARN(
                            "Task {}: Consume() attacker_ip_merge returns NULL, wait for "
                            "input ...",
                            name_);
                }
                auto attacker_ip_merge = *(attacker_ip_merge_opt.value());
                SPDLOG_DEBUG("Task {}: attacker_ip_merge: {}", name_, attacker_ip_merge);

                // input 2, host nic name of the packets
                Datum host_nic_name_d = ctx->Inputs()[2].Value();
                SPDLOG_DEBUG(
                    "Task {}: Consume host_nic_name: {}", name_, host_nic_name_d);
                auto host_nic_name_opt = host_nic_name_d.Consume<std::string>();
                if (!host_nic_name_opt)
                {
                        SPDLOG_WARN(
                            "Task {}: Consume() host_nic_name returns NULL, wait for "
                            "input ...",
                            name_);
                }
                auto host_nic_name = *(host_nic_name_opt.value());
                SPDLOG_DEBUG("Task {}: host_nic_name: {}", name_, host_nic_name);

                // number stats
                // sidedata0, number ratio min
                auto num_ratioMin_d = ctx->GetInputSideData()[0];
                SPDLOG_DEBUG(
                    "Task {}: Consume side data num_ratioMin: {}", name_, num_ratioMin_d);

                auto num_ratioMin_opt = num_ratioMin_d.Consume<double_t>();
                if (!num_ratioMin_opt)
                {
                        SPDLOG_WARN(
                            "Task {}: Consume() num_ratioMin returns NULL, wait "
                            "for "
                            "input ...",
                            name_);
                }

                auto num_ratioMin = *(num_ratioMin_opt.value());
                SPDLOG_DEBUG("Task {}: num_ratioMin: {}", name_, num_ratioMin);

                // sidedata1, number ratio max
                auto num_ratioMax_d = ctx->GetInputSideData()[1];
                SPDLOG_DEBUG(
                    "Task {}: Consume side data num_ratioMax: {}", name_, num_ratioMax_d);

                auto num_ratioMax_opt = num_ratioMax_d.Consume<double_t>();
                if (!num_ratioMax_opt)
                {
                        SPDLOG_WARN(
                            "Task {}: Consume() num_ratioMax returns NULL, wait "
                            "for "
                            "input ...",
                            name_);
                }

                auto num_ratioMax = *(num_ratioMax_opt.value());
                SPDLOG_DEBUG("Task {}: num_ratioMax: {}", name_, num_ratioMax);

                // sidedata2, number stat type
                auto num_stat_type_d = ctx->GetInputSideData()[2];
                SPDLOG_DEBUG(
                    "Task {}: Consume side data num_stat_type: {}",
                    name_,
                    num_stat_type_d);

                auto num_stat_type_opt =
                    num_stat_type_d.Consume<std::vector<std::string>>();
                if (!num_stat_type_opt)
                {
                        SPDLOG_WARN(
                            "Task {}: Consume() num_stat_type returns NULL, wait "
                            "for "
                            "input ...",
                            name_);
                }

                auto num_stat_type = *(num_stat_type_opt.value());
                SPDLOG_DEBUG("Task {}: num_stat_type: {}", name_, num_stat_type);

                // proto stats
                // sidedata3, proto ratio min
                auto proto_ratioMin_d = ctx->GetInputSideData()[3];
                SPDLOG_DEBUG(
                    "Task {}: Consume side data proto_ratioMin: {}",
                    name_,
                    proto_ratioMin_d);

                auto proto_ratioMin_opt = proto_ratioMin_d.Consume<double_t>();
                if (!proto_ratioMin_opt)
                {
                        SPDLOG_WARN(
                            "Task {}: Consume() proto_ratioMin returns NULL, "
                            "wait for "
                            "input ...",
                            name_);
                }

                auto proto_ratioMin = *(proto_ratioMin_opt.value());
                SPDLOG_DEBUG("Task {}: proto_ratioMin: {}", name_, proto_ratioMin);

                // sidedata4, proto ratio max
                auto proto_ratioMax_d = ctx->GetInputSideData()[4];
                SPDLOG_DEBUG(
                    "Task {}: Consume side data proto_ratioMax: {}",
                    name_,
                    proto_ratioMax_d);

                auto proto_ratioMax_opt = proto_ratioMax_d.Consume<double_t>();
                if (!proto_ratioMax_opt)
                {
                        SPDLOG_WARN(
                            "Task {}: Consume() proto_ratioMax returns NULL, "
                            "wait for "
                            "input ...",
                            name_);
                }

                auto proto_ratioMax = *(proto_ratioMax_opt.value());
                SPDLOG_DEBUG("Task {}: proto_ratioMax: {}", name_, proto_ratioMax);

                // sidedata5, proto stat type
                auto proto_stat_type_d = ctx->GetInputSideData()[5];
                SPDLOG_DEBUG(
                    "Task {}: Consume side data proto_stat_type: {}",
                    name_,
                    proto_stat_type_d);

                auto proto_stat_type_opt =
                    proto_stat_type_d.Consume<std::vector<std::string>>();
                if (!proto_stat_type_opt)
                {
                        SPDLOG_WARN(
                            "Task {}: Consume() proto_stat_type returns NULL, "
                            "wait for "
                            "input ...",
                            name_);
                }

                auto proto_stat_type = *(proto_stat_type_opt.value());
                SPDLOG_DEBUG("Task {}: proto_stat_type: {}", name_, proto_stat_type);

                // sip based merge
                std::vector<std::string> cidr_string_vec;
                std::map<std::string, CIDR> convert_map;
                for (auto&& ip : attacker_ip_merge.attackerIPs)
                {
                        cidr_string_vec.emplace_back(std::move(
                            std::to_string(ip.ip) + "/" + std::to_string(ip.len)));

                        convert_map[cidr_string_vec.back()] = ip;
                }

                // key will be CIDR, tmp use string now, string is "uint32/len"
                std::unordered_map<std::string, snding::SIPBaseMergeResult> all_merge_ret;

                // sip based merge, step 1, non-random merge
                // sip cidr based merge of all packets
                // packet: std::unordered_map<std::string, uint32_t>

                // ip_positions is for optimize in random ip merge
                // key: ip, value: positions of this ip
                std::unordered_map<uint32_t, std::vector<int>> ip_positions;
                int index = 0;
                for (auto&& packet : packets)
                {
                        for (size_t i = 0; i < attacker_ip_merge.attackerIPs.size(); ++i)
                        {
                                if (belongs(
                                        attacker_ip_merge.attackerIPs[i], packet["SIP"]))
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

                                        break;
                                }
                        }

                        auto& pos_vec = ip_positions[packet["SIP"]];
                        pos_vec.push_back(index);
                        index++;
                }

                // sip based merge, step 2, random merge
                if (attacker_ip_merge.containRandomAttack)
                {
                        SPDLOG_DEBUG("Task {}: with random IPs", name_);

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
                            numValueSum,
                            num_ratioMin,
                            num_ratioMax,
                            num_stat_type);

                        auto dport_stat_type = calc_number_stat_type(
                            merge_ret.second.dport,
                            numValueSum,
                            num_ratioMin,
                            num_ratioMax,
                            num_stat_type);

                        auto length_stat_type = calc_number_stat_type(
                            merge_ret.second.length,
                            numValueSum,
                            num_ratioMin,
                            num_ratioMax,
                            num_stat_type);

                        // for sport, dport, length, if the stat_type is
                        // CENTRALIZED, use merge_ret.second.sport/dport/length as
                        // the merge result for next step; otherwise, use the
                        // stat_type name shown in the next step.
                        stat.srcPort.stat_type = sport_stat_type;
                        if (sport_stat_type == num_stat_type[0])
                        {
                                stat.srcPort.value = std::move(merge_ret.second.sport);
                        }
                        stat.dstPort.stat_type = dport_stat_type;
                        if (dport_stat_type == num_stat_type[0])
                        {
                                stat.dstPort.value = std::move(merge_ret.second.dport);
                        }
                        stat.length.stat_type = length_stat_type;
                        if (length_stat_type == num_stat_type[0])
                        {
                                stat.length.value = std::move(merge_ret.second.length);
                        }

                        // process protocol
                        auto prot = calc_proto_stat_type(
                            merge_ret.second.protocol,
                            numValueSum,
                            proto_ratioMin,
                            proto_ratioMax,
                            proto_stat_type);
                        stat.protocol.stat_types = std::move(prot);

                        if (merge_ret.first == "RANDOM")
                        {
                                stat.isSrcIPRandom = true;
                                // downstream node will use "RANDOM-dstip"
                                // to match attack link.
                                stat.dstIP.value = std::move(merge_ret.second.dstIP);
                        }
                        else
                        {
                                // downstream node will use "srcip-dstip"
                                // to match attack link.
                                stat.srcIP = convert_map[merge_ret.first];
                                // stat.dstIP = {packets.at(0)["DIP"], 32};
                                stat.dstIP.value = std::move(merge_ret.second.dstIP);
                        }
                }

                SPDLOG_DEBUG("Task {}: after calculation: {}", name_, all_stat);

                ctx->Outputs()[0].AddDatum(Datum(std::move(all_stat)));

                return 0;
        }

        // 2nd arg `ip` actually is CIDR{ip, 32}
        // `ip`为`cidr`的匹配集的子集等价于:
        // `cidr`的前缀长度不大于32的且`cidr`和`ip`的前`cidr`.len位完全相同
        bool SndSIPBaseMergeTask::belongs(CIDR cidr, uint32_t ip)
        {
                if (cidr.len > 32)
                        return false;
                if ((cidr.ip ^ ip) >> (32 - cidr.len))
                        return false;
                return true;
        }

        std::string SndSIPBaseMergeTask::calc_number_stat_type(
            const std::unordered_set<uint32_t>& uset,
            int numValueSum,
            double_t num_ratioMin,
            double_t num_ratioMax,
            const std::vector<std::string>& num_stat_type)
        {
                // numKeyLen
                auto numKeyLen = uset.size();

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
                        //     "Task {}: keyDiffs.insert: {}", name_, keys[i + 1]
                        //     - keys[i]);
                }
                auto keySeriesDiffTypeNum = keyDiffs.size();

                SPDLOG_DEBUG(
                    "Task {}: keySeriesDiffTypeNum: {}", name_, keySeriesDiffTypeNum);

                std::string stat_type;
                double_t score = 0.0;
                double_t min_ = (double_t) numValueSum * num_ratioMin;
                double_t max_ = (double_t) numValueSum * num_ratioMax;
                if (numKeyLen < min_)
                {
                        stat_type = num_stat_type[0];
                }
                else
                {
                        if (keySeriesDiffTypeNum <= min_)
                        {
                                stat_type = num_stat_type[1];
                        }
                        else if (keySeriesDiffTypeNum >= max_)
                        {
                                stat_type = num_stat_type[2];
                        }
                        else
                        {
                                stat_type = num_stat_type[3];
                        }
                }

                SPDLOG_DEBUG("Task {}: num stat type: {}", name_, stat_type);

                return stat_type;
        }

        std::unordered_map<int, std::string> SndSIPBaseMergeTask::calc_proto_stat_type(
            const std::unordered_map<int, int>& proto_map,
            int numValueSum,
            double_t proto_ratioMin,
            double_t proto_ratioMax,
            const std::vector<std::string>& proto_stat_type)
        {
                SPDLOG_DEBUG("Task {}: numValueSum: {}", name_, numValueSum);
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
                            "Task {}: protocol: {}, protoCountValue: {}, proto "
                            "stat "
                            "type: {}",
                            name_,
                            proto.first,
                            protoCountValue,
                            stat_type);

                        stats[proto.first] = stat_type;
                }

                return std::move(stats);
        }

        REGISTER(SndSIPBaseMergeTask);

}   // namespace dni
