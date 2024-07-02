#include "dni_client.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>
#include <vector>

#include "spdlog/spdlog.h"

void DNIClient::CalculateGraph(const std::string& fpath)
{
        // std::ifstream fin(fpath);
        // if (!fin.is_open())
        // {
        //         SPDLOG_CRITICAL("can't open {}", fpath);
        //         return;
        // }

        // std::stringstream ss;
        // ss << fin.rdbuf();

        ClientContext context;
        buildHeaders(context);
        GraphResponse resp;
        GraphRequest req;

        req.set_pbtxt("pbtxt");
        for (size_t i = 0; i < 10; i++)
        {
                req.add_all_ips(0x01020304 + i);
        }
        req.add_all_ips(0xC0A80F56);
        req.add_all_ips(0xC0A81056);
        req.add_all_ips(0xC0A8103C);
        req.add_all_ips(0xC0A80F3C);

        req.set_all_nic_number(6);
        req.set_count_total_threshold(1000);

        const std::unique_ptr<::grpc::ClientReader<GraphResponse>>& response(
            client->CalculateGraph(&context, req));
        while (response->Read(&resp))
        {
                SPDLOG_DEBUG("Receive one result from dni ...");
                printGraphResponse(context, resp);
        }
}

void DNIClient::printGraphResponse(ClientContext& context, const GraphResponse& response)
{
        const multimap<grpc::string_ref, grpc::string_ref>& headers =
            context.GetServerInitialMetadata();
        for (const auto& header : headers)
        {
                SPDLOG_DEBUG(
                    "<-H {}: {}",
                    std::string(header.first.begin(), header.first.end()),
                    std::string(header.second.begin(), header.second.end()));
        }

        const RepeatedPtrField<GraphCalcResult>& results = response.results();
        for (const GraphCalcResult& result : results)
        {
                uint64_t ts = result.ts();
                auto ret_type = result.type();
                auto host_nic_name = result.host_nic_name();

                std::string str_ret = "";
                str_ret +=
                    ("graph result, ts: " + std::to_string(ts) +
                     ", type: " + std::to_string((int) ret_type) +
                     ", host_nic_name: " + host_nic_name + "\n");

                str_ret +=
                    ("graph1 result, abnormal_type: " +
                     std::to_string(result.abnormal_type()) +
                     ", graph2 result, attack_type: " +
                     std::to_string(result.attack_type()) + "\n");

                str_ret +=
                    ("# single_nic_analysis inMbps: *" + std::to_string(result.inmbps()) +
                     "*\n");
                str_ret +=
                    ("# single_nic_analysis outMbps: *" +
                     std::to_string(result.outmbps()) + "*\n");
                str_ret +=
                    ("# single_nic_analysis dmsDropMbps: *" +
                     std::to_string(result.dmsdropmbps()) + "*\n");
                str_ret +=
                    ("# single_nic_analysis speedStr: *" +
                     std::to_string(result.speed()) + "Mb/s*\n");
                str_ret +=
                    ("# single_nic_analysis cur_cpu: *" +
                     std::to_string(result.cur_cpu()) + "*\n");
                str_ret +=
                    ("# single_nic_analysis result: *" + result.detect_result() + "*\n");

                std::string str_rules = "";

                SPDLOG_INFO("rule size: {}", result.rules().size());
                for (const auto& rule : result.rules())
                {
                        str_rules += ("[" + fmt::to_string(rule) + "]\n");
                }

                str_ret += ("# single_nic_analysis detail: *{" + str_rules + "}*");

                SPDLOG_INFO("{}", str_ret);
        }

        const multimap<grpc::string_ref, grpc::string_ref>& tails =
            context.GetServerTrailingMetadata();
        for (const auto& tail : tails)
        {
                SPDLOG_DEBUG(
                    "<-L {} : {}",
                    std::string(tail.first.begin(), tail.first.end()),
                    std::string(tail.second.begin(), tail.second.end()));
        }

        SPDLOG_DEBUG("\n");
}

void DNIClient::buildHeaders(ClientContext& context)
{
        // examples/cpp/metadata
        context.AddMetadata("k1", "v1");
        context.AddMetadata("k2", "v2");
}

namespace fmt {

        template <>
        struct formatter<GraphDMSRule>: formatter<std::string_view> {
                auto format(const GraphDMSRule& rule, format_context& ctx) const
                {
                        return format_to(
                            ctx.out(),
                            "packet_ts: {}, hostNicSign: {}\n, srcIP: {:X}/{}\t, dstIP: "
                            "{:X}/{}\n,"
                            "sPort: {}\t, dPort: {}\t, length: {}\t, protocol: {}\n"
                            "action: {}\t, limitMode: {}\t, limitMaxValue: {}\n\n",
                            rule.packets_ts(), rule.hostnicsign(), rule.srcip(),
                            rule.srcip_len(), rule.dstip(), rule.dstip_len(),
                            rule.sport(), rule.dport(), rule.length(), rule.protocol(),
                            rule.action(), rule.limitmode(), rule.limitmaxvalue());
                }
        };
}   // namespace fmt
