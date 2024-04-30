#include <string>
#include <unordered_map>
#include <unordered_set>

#include "dni/framework/framework.h"
#include "snding_defines.h"
#include "spdlog/spdlog.h"

namespace dni {

        class SIPBaseMergeTask: public TaskBase {
        public:
                SIPBaseMergeTask(): name_("SIPBaseMergeTask") {}
                ~SIPBaseMergeTask() override {}

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
                    float num_ratioMin,
                    float num_ratioMax,
                    const std::vector<std::string>& num_stat_type);

                std::unordered_map<int, std::string> calc_proto_stat_type(
                    const std::unordered_map<int, int>& proto_map,
                    int numValueSum,
                    float proto_ratioMin,
                    float proto_ratioMax,
                    const std::vector<std::string>& proto_stat_type);

                bool belongs(IP cidr, uint32_t ip);
        };

        int SIPBaseMergeTask::Process(TaskContext* ctx)
        {
                // input0, packets
                Datum packets_d = ctx->Inputs()[0].Value();
                SPDLOG_DEBUG("Task {}: Consume Datum: {}", name_, packets_d);
                auto packets_opt = packets_d.Consume<
                    std::vector<std::unordered_map<std::string, uint32_t>>>();
                if (!packets_opt)
                {
                        SPDLOG_WARN(
                            "Task {}: Consume() returns NULL, wait for "
                            "input ...",
                            name_);
                }
                auto packets = *(packets_opt.value());
                // SPDLOG_DEBUG("Task {}: packets: {}", name_, packets);

                // input1, cidr merge result
                Datum sip_array_d = ctx->Inputs()[1].Value();
                SPDLOG_DEBUG("Task {}: Consume Datum: {}", name_, sip_array_d);
                auto sip_array_opt = sip_array_d.Consume<std::vector<IP>>();
                if (!sip_array_opt)
                {
                        SPDLOG_WARN(
                            "Task {}: Consume() returns NULL, wait for "
                            "input ...",
                            name_);
                }
                auto sip_array = *(sip_array_opt.value());
                SPDLOG_DEBUG("Task {}: sip_array: {}", name_, sip_array);

                // sidedata0, number ratio min
                auto num_ratioMin_d = ctx->GetInputSideData()[0];
                SPDLOG_DEBUG(
                    "Task {}: Consume side data num_ratioMin_d: {}",
                    name_,
                    num_ratioMin_d);

                auto num_ratioMin_opt = num_ratioMin_d.Consume<float_t>();
                if (!num_ratioMin_opt)
                {
                        SPDLOG_WARN(
                            "Task {}: Consume() returns NULL, wait for "
                            "input ...",
                            name_);
                }

                auto num_ratioMin = *(num_ratioMin_opt.value());
                SPDLOG_DEBUG("Task {}: num_ratioMin: {}", name_, num_ratioMin);

                // sidedata1, number ratio max
                auto num_ratioMax_d = ctx->GetInputSideData()[1];
                SPDLOG_DEBUG(
                    "Task {}: Consume side data num_ratioMax_d: {}",
                    name_,
                    num_ratioMax_d);

                auto num_ratioMax_opt = num_ratioMax_d.Consume<float_t>();
                if (!num_ratioMax_opt)
                {
                        SPDLOG_WARN(
                            "Task {}: Consume() returns NULL, wait for "
                            "input ...",
                            name_);
                }

                auto num_ratioMax = *(num_ratioMax_opt.value());
                SPDLOG_DEBUG("Task {}: num_ratioMax: {}", name_, num_ratioMax);

                // sidedata2, number stat enum
                auto num_stat_type_d = ctx->GetInputSideData()[2];
                SPDLOG_DEBUG(
                    "Task {}: Consume side data num_stat_type_d: {}",
                    name_,
                    num_stat_type_d);

                auto num_stat_type_opt =
                    num_stat_type_d.Consume<std::vector<std::string>>();
                if (!num_stat_type_opt)
                {
                        SPDLOG_WARN(
                            "Task {}: Consume() returns NULL, wait for "
                            "input ...",
                            name_);
                }

                auto num_stat_type = *(num_stat_type_opt.value());
                SPDLOG_DEBUG("Task {}: num_stat_type: {}", name_, num_stat_type);

                // proto
                // sidedata3, proto ratio min
                auto proto_ratioMin_d = ctx->GetInputSideData()[3];
                SPDLOG_DEBUG(
                    "Task {}: Consume side data proto_ratioMin_d: {}",
                    name_,
                    proto_ratioMin_d);

                auto proto_ratioMin_opt = proto_ratioMin_d.Consume<float_t>();
                if (!proto_ratioMin_opt)
                {
                        SPDLOG_WARN(
                            "Task {}: Consume() returns NULL, wait for "
                            "input ...",
                            name_);
                }

                auto proto_ratioMin = *(proto_ratioMin_opt.value());
                SPDLOG_DEBUG("Task {}: proto_ratioMin: {}", name_, proto_ratioMin);

                // sidedata4, proto ratio max
                auto proto_ratioMax_d = ctx->GetInputSideData()[4];
                SPDLOG_DEBUG(
                    "Task {}: Consume side data proto_ratioMax_d: {}",
                    name_,
                    proto_ratioMax_d);

                auto proto_ratioMax_opt = proto_ratioMax_d.Consume<float_t>();
                if (!proto_ratioMax_opt)
                {
                        SPDLOG_WARN(
                            "Task {}: Consume() returns NULL, wait for "
                            "input ...",
                            name_);
                }

                auto proto_ratioMax = *(proto_ratioMax_opt.value());
                SPDLOG_DEBUG("Task {}: proto_ratioMax: {}", name_, proto_ratioMax);

                // sidedata5, proto stat enum
                auto proto_stat_type_d = ctx->GetInputSideData()[5];
                SPDLOG_DEBUG(
                    "Task {}: Consume side data proto_stat_type_d: {}",
                    name_,
                    proto_stat_type_d);

                auto proto_stat_type_opt =
                    proto_stat_type_d.Consume<std::vector<std::string>>();
                if (!proto_stat_type_opt)
                {
                        SPDLOG_WARN(
                            "Task {}: Consume() returns NULL, wait for "
                            "input ...",
                            name_);
                }

                auto proto_stat_type = *(proto_stat_type_opt.value());
                SPDLOG_DEBUG("Task {}: proto_stat_type: {}", name_, proto_stat_type);

                std::vector<std::string> cidr_string_vec;
                std::map<std::string, IP> convert_map;
                for (auto&& ip : sip_array)
                {
                        cidr_string_vec.emplace_back(std::move(
                            std::to_string(ip.ip) + "/" + std::to_string(ip.len)));

                        convert_map[cidr_string_vec.back()] = ip;
                }

                // key will be IP, tmp use string now
                std::unordered_map<std::string, SIPBaseMergeResult> all_merge_ret;

                // sip cidr based merge of all packets
                // packet: std::unordered_map<std::string, uint32_t>
                for (auto&& packet : packets)
                {
                        for (size_t i = 0; i < sip_array.size(); ++i)
                        {
                                if (belongs(sip_array[i], packet["SIP"]))
                                {
                                        SIPBaseMergeResult& ret =
                                            all_merge_ret[cidr_string_vec[i]];
                                        ret.packet_total++;
                                        ret.sport.insert(packet["SPort"]);
                                        ret.dport.insert(packet["DPort"]);
                                        ret.length.insert(packet["Length"]);

                                        // ret.protocol.insert(packet["Protocol"]);
                                        int& count = ret.protocol[packet["Protocol"]];
                                        count++;

                                        break;
                                }
                        }
                }

                // key will be IP, tmp use string now
                // analyze the situation of sport/dport/length/protocol
                std::unordered_map<std::string, SIPBaseMergeStats> all_stat;
                for (auto&& merge_ret : all_merge_ret)
                {
                        // process number
                        SIPBaseMergeStats& stat = all_stat[merge_ret.first];
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

                        // for sport, dport, length, if the stat_type is CENTRALIZED,
                        // use merge_ret.second.sport/dport/length as the merge result for
                        // next step; otherwise, use the stat_type name shown in the next
                        // step.
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

                        stat.srcIP = convert_map[merge_ret.first];
                        stat.dstIP = {packets.at(0)["DIP"], 32};
                }

                SPDLOG_DEBUG("Task {}: after calculation: {}", name_, all_stat);

                ctx->Outputs()[0].AddDatum(Datum(all_stat));

                return 0;
        }

        // 2nd arg `ip` actually is IP{ip, 32}
        // `ip`为`cidr`的匹配集的子集等价于:
        // `cidr`的前缀长度不大于32的且`cidr`和`ip`的前`cidr`.len位完全相同
        bool SIPBaseMergeTask::belongs(IP cidr, uint32_t ip)
        {
                if (cidr.len > 32)
                        return false;
                if ((cidr.ip ^ ip) >> (32 - cidr.len))
                        return false;
                return true;
        }

        std::string SIPBaseMergeTask::calc_number_stat_type(
            const std::unordered_set<uint32_t>& uset,
            int numValueSum,
            float num_ratioMin,
            float num_ratioMax,
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
                        //     "Task {}: keyDiffs.insert: {}", name_, keys[i + 1] -
                        //     keys[i]);
                }
                auto keySeriesDiffTypeNum = keyDiffs.size();

                SPDLOG_DEBUG(
                    "Task {}: keySeriesDiffTypeNum: {}", name_, keySeriesDiffTypeNum);

                std::string stat_type;
                float_t score = 0.0;
                float_t min_ = (float_t) numValueSum * num_ratioMin;
                float_t max_ = (float_t) numValueSum * num_ratioMax;
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

        std::unordered_map<int, std::string> SIPBaseMergeTask::calc_proto_stat_type(
            const std::unordered_map<int, int>& proto_map,
            int numValueSum,
            float proto_ratioMin,
            float proto_ratioMax,
            const std::vector<std::string>& proto_stat_type)
        {
                SPDLOG_DEBUG("Task {}: numValueSum: {}", name_, numValueSum);
                std::unordered_map<int, std::string> stats;
                float_t min_ = (float_t) numValueSum * proto_ratioMin;
                float_t max_ = (float_t) numValueSum * proto_ratioMax;

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
                            "Task {}: protocol: {}, protoCountValue: {}, proto stat "
                            "type: {}",
                            name_,
                            proto.first,
                            protoCountValue,
                            stat_type);

                        stats[proto.first] = stat_type;
                }

                return std::move(stats);
        }

        REGISTER(SIPBaseMergeTask);

}   // namespace dni
