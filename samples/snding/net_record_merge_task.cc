#include "dni/framework/framework.h"
#include "spdlog/spdlog.h"
#include "snding_defines.h"

namespace dni {

        class NetRecordMergeTask: public TaskBase {
        public:
                NetRecordMergeTask(): name_("NetRecordMergeTask") {}
                ~NetRecordMergeTask() override {}

                int Open(TaskContext* ctx) override
                {
                        SPDLOG_DEBUG("Task {}: open task ...", name_);

                        return 0;
                }

                int Process(TaskContext* ctx) override
                {
                        // input
                        Datum d = ctx->Inputs()[0].Value();
                        SPDLOG_DEBUG("Task {}: Consume Datum: {}", name_, d);
                        auto opt = d.Consume<std::vector<std::unordered_map<std::string, SIPBaseMergeStats>>>();
                        if (!opt)
                        {
                                SPDLOG_WARN(
                                    "Task {}: Consume() returns NULL, wait for "
                                    "input ...",
                                    name_);
                        }
                        auto records = *(opt.value());
                        SPDLOG_DEBUG("Task {}: val: {}", name_, records);

                        // match
                        // key: srcip-cidr:dstip/32
                        std::unordered_map<std::string, std::unordered_set<std::string>> hosts_in_attack_link;
                        for (auto &&net_records : records)
                        {
                                for (auto &&record : net_records)
                                {
                                        auto key = record.first + ":" + std::to_string(record.second.dstIP.ip) + "/32";
                                        auto& attack_link = hosts_in_attack_link[key];
                                        attack_link.insert(record.second.hostNicSign);
                                }
                        }

                        SPDLOG_DEBUG("Task {}: after calculation: {}", name_, hosts_in_attack_link);

                        ctx->Outputs()[0].AddDatum(Datum(hosts_in_attack_link));

                        return 0;
                }

                int Close(TaskContext* ctx) override
                {
                        SPDLOG_DEBUG("Task {}: closing ...", name_);

                        return 0;
                }

        private:
                std::string name_;
        };

        REGISTER(NetRecordMergeTask);

}   // namespace dni
