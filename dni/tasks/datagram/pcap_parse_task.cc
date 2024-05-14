#include <string>
#include <unordered_map>
#include <vector>

#include "daq.h"
#include "dni/framework/framework.h"
#include "spdlog/spdlog.h"

namespace dni {

// TODO: Add options for parsing live packets or offline pcap file.
class PcapParseTask: public TaskBase {
public:
        PcapParseTask(): name_("PcapParseTask") {}
        ~PcapParseTask() override {}

        int Open(TaskContext* ctx) override
        {
                name_ += "(" + ctx->Name() + ")";
                SPDLOG_DEBUG("{}: open task ...", name_);

                return 0;
        }

        int Process(TaskContext* ctx) override
        {
                std::vector<std::unordered_map<std::string, uint32_t>> packets;

                Datum pcap_d = ctx->Inputs()[0].Value();
                SPDLOG_DEBUG("{}: Consume Datum: {}", name_, pcap_d);
                auto pcap_opt = pcap_d.Consume<std::string>();
                if (!pcap_opt)
                {
                        SPDLOG_CRITICAL("{}: invalid pcap path", name_);
                        return -1;
                }
                auto pcap = *(pcap_opt.value());
                SPDLOG_DEBUG("{}: pcap: {}", name_, pcap);

                // parse

                SPDLOG_DEBUG("loading pcap file {} ...", pcap);
                char errbuf[PCAP_ERRBUF_SIZE];

                pcap_t* handle = pcap_open_offline(pcap.c_str(), errbuf);
                if (handle == NULL)
                {
                        SPDLOG_ERROR(
                            "fail to execute pcap_open_offline({}), error is {}",
                            pcap,
                            errbuf);
                        return -1;
                }
                pcap_activate(handle);
                pcap_loop(handle, -1, parse_single_packet, (u_char*) &packets);

                SPDLOG_DEBUG("{}: packets size: {}", name_, packets.size());

                ctx->Outputs()[0].AddDatum(Datum(std::move(packets)));

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

REGISTER(PcapParseTask);

}   // namespace dni
