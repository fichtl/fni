#include "dni_client.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>
#include <vector>

#include "spdlog/spdlog.h"

void DNIClient::CalculateGraph(const std::string& fpath)
{
        std::ifstream fin(fpath);
        if (!fin.is_open())
        {
                SPDLOG_CRITICAL("can't open {}", fpath);
                return;
        }

        std::stringstream ss;
        ss << fin.rdbuf();

        ClientContext context;
        buildHeaders(context);
        GraphResponse resp;
        GraphRequest req;

        req.set_pbtxt(ss.str());
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
                SPDLOG_DEBUG(
                    "graph result, ts: {}, type: {}, and values:", ts, (int) ret_type);

                const Map<std::string, std::string>& out_pairs = result.results();
                for (auto iter = out_pairs.cbegin(); iter != out_pairs.cend(); iter++)
                {
                        SPDLOG_DEBUG("    {}: {}", iter->first, iter->second);
                }
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
