#include "dni/framework/framework.h"
#include "spdlog/spdlog.h"

namespace dni {

// Inputs: packet/netdev/resource result
//  vector of scores (double)
// Outputs:
//  -1: input wrong;
//  0: normal
//  1~4: abnormal
class SndAdTask: public TaskBase {
public:
        SndAdTask(): name_("SndAdTask") {}
        ~SndAdTask() override {}

        int Open(TaskContext* ctx) override
        {
                name_ += "(" + ctx->Name() + ")";
                SPDLOG_DEBUG("{}: open task ...", name_);

                return 0;
        }

        int Process(TaskContext* ctx) override
        {
                if (ctx->Inputs().size() != 3)
                {
                        SPDLOG_CRITICAL(
                            "{}: input size is not 3, is: {}",
                            name_,
                            ctx->Inputs().size());
                        return -1;
                }

                int sum = 0;

                Datum pkt_d = ctx->Inputs().Tag("PACKET").Value();
                SPDLOG_DEBUG("{}: Consume Datum: {}", name_, pkt_d);
                auto pkt_opt = pkt_d.Consume<double_t>();
                if (!pkt_opt)
                {
                        SPDLOG_CRITICAL("{}: invalid input", name_);
                        return -1;
                }
                auto pkt = *(pkt_opt.value());
                SPDLOG_DEBUG("{}: pkt: {}", name_, pkt);
                SPDLOG_INFO("{}: pkt: {}", name_, (int(pkt + 0.5)));
                sum += (int(pkt + 0.5));

                Datum netdev_d = ctx->Inputs().Tag("NETDEV").Value();
                SPDLOG_DEBUG("{}: Consume Datum: {}", name_, netdev_d);
                auto netdev_opt = netdev_d.Consume<double_t>();
                if (!netdev_opt)
                {
                        SPDLOG_CRITICAL("{}: invalid input", name_);
                        return -1;
                }
                auto netdev = *(netdev_opt.value());
                SPDLOG_DEBUG("{}: netdev: {}", name_, netdev);
                SPDLOG_INFO("{}: pkt: {}", name_, (int(netdev + 0.5)));
                sum += (int(netdev + 0.5));

                Datum resource_d = ctx->Inputs().Tag("RESOURCE").Value();
                SPDLOG_DEBUG("{}: Consume Datum: {}", name_, resource_d);
                auto resource_opt = resource_d.Consume<double_t>();
                if (!resource_opt)
                {
                        SPDLOG_CRITICAL("{}: invalid input", name_);
                        return -1;
                }
                auto resource = *(resource_opt.value());
                SPDLOG_DEBUG("{}: resource: {}", name_, resource);
                SPDLOG_INFO("{}: pkt: {}", name_, (int(resource + 0.5)));
                sum += (int(resource + 0.5));

                int abnormal_type = 0;   // 0, normal
                if (sum == 3)
                {
                        abnormal_type = 1;
                }
                else if (sum == 2)
                {
                        if (pkt + netdev == 2)
                        {
                                abnormal_type = 2;
                        }
                        else if (pkt + resource == 2)
                        {
                                abnormal_type = 3;
                        }
                        else if (netdev + resource == 2)
                        {
                                abnormal_type = 4;
                        }
                }

                SPDLOG_DEBUG("{}: after calculation: {}", name_, abnormal_type);

                ctx->Outputs()[0].AddDatum(Datum(abnormal_type));

                return 0;
        }

        int Close(TaskContext* ctx) override
        {
                SPDLOG_DEBUG("{}: closing ...", name_);

                return 0;
        }

private:
        std::string name_;
};

REGISTER(SndAdTask);

}   // namespace dni
