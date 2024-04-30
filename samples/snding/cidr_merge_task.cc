#include <list>

#include "dni/framework/framework.h"
#include "snding_defines.h"
#include "spdlog/spdlog.h"

namespace dni {

        class CIDRMergeTask: public TaskBase {
        public:
                CIDRMergeTask(): name_("CIDRMergeTask") {}
                ~CIDRMergeTask() override {}

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
                std::string name_;

                bool isChildCollection(IP& a, IP& b);
                void merge1(std::list<IP>& ipl);
                bool unionCollection(IP& a, IP& b);
                void merge2(std::list<IP>& ipl);
        };

        int CIDRMergeTask::Process(TaskContext* ctx)
        {
                // input
                Datum ip_map_d = ctx->Inputs()[0].Value();
                SPDLOG_DEBUG("Task {}: Consume Datum: {}", name_, ip_map_d);
                auto ip_map_opt = ip_map_d.Consume<std::unordered_map<uint32_t, int>>();
                if (!ip_map_opt)
                {
                        SPDLOG_WARN(
                            "Task {}: Consume() returns NULL, wait for "
                            "input ...",
                            name_);
                }
                auto ip_map = *(ip_map_opt.value());
                SPDLOG_DEBUG("Task {}: val: {}", name_, ip_map);

                // merge
                std::list<IP> ipl;
                for (auto&& ip : ip_map)
                {
                        ipl.emplace_back(ip.first, 32);
                }

                // for (auto it = ipl.begin(); it != ipl.end(); it++)
                // {
                //         printf("%x/%d\n", it->ip, it->len);
                // }

                ipl.sort([](const IP& a, const IP& b) {
                        if (a.ip != b.ip)
                                return a.ip < b.ip;
                        return a.len < b.len;
                });
                merge1(ipl);
                merge2(ipl);

                SPDLOG_DEBUG("Task {}: after calculation: {}", name_, ipl.size());

                std::vector<IP> ret;
                for (auto&& ip : ipl)
                {
                        ret.emplace_back(std::move(ip));
                }

                ctx->Outputs()[0].AddDatum(Datum(ret));

                return 0;
        }

        // 根据题意b的匹配集为a的匹配集的子集等价于
        //  a的前缀长度不大于b的且a和b的ip地址的前a.len位完全相同
        bool CIDRMergeTask::isChildCollection(IP& a, IP& b)
        {
                if (a.len > b.len)
                        return false;
                if ((a.ip ^ b.ip) >> (32 - a.len))
                        return false;
                return true;
        }

        void CIDRMergeTask::merge1(std::list<IP>& ipl)
        {
                auto i = ipl.begin(), j = ipl.begin();
                j++;
                while (j != ipl.end())
                {
                        if (isChildCollection(*i, *j))
                                j = ipl.erase(j);
                        else
                        {
                                i++;
                                j++;
                        }
                }
        }

        // 根据题意a与b为a`可以合并等价于a与b前缀长度相同且二者的前len位中只有最后一位不相同
        bool CIDRMergeTask::unionCollection(IP& a, IP& b)
        {
                if (a.len != b.len)
                        return false;
                return ((a.ip ^ b.ip) >> (32 - a.len)) == 1u;
        }

        void CIDRMergeTask::merge2(std::list<IP>& ipl)
        {
                auto i = ipl.begin(), j = ipl.begin();
                j++;
                while (j != ipl.end())
                {
                        if (unionCollection(*i, *j))
                        {
                                j = ipl.erase(j);
                                ((*i).len)--;
                                if (i != ipl.begin())
                                {
                                        i--;
                                        j--;
                                }
                        }
                        else
                        {
                                i++;
                                j++;
                        }
                }
        }

        REGISTER(CIDRMergeTask);
}   // namespace dni
