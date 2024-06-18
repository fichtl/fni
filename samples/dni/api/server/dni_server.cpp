#include "dni_server.h"

#include <chrono>
#include <thread>

#include "dni/framework/framework.h"
#include "spdlog/spdlog.h"

Status DNIServiceImpl::CalculateGraph(
    ServerContext* context, const GraphRequest* request,
    ServerWriter<GraphResponse>* writer)
{
        printHeaders(context);
        if (client != nullptr)
        {
                grpc::ClientContext c;
                propagateHeaders(context, c);
                GraphResponse graphResponse;
                const std::unique_ptr<::grpc::ClientReader<GraphResponse>>& response(
                    client->CalculateGraph(&c, *request));
                while (response->Read(&graphResponse))
                {
                        writer->Write(graphResponse);
                }
        }
        else
        {
                const std::string& pbtxt = request->pbtxt();
                SPDLOG_DEBUG("CalculateGraph REQUEST: data={}", pbtxt);

                auto gc = dni::ParseStringToGraphConfig(pbtxt);
                if (!gc)
                {
                        SPDLOG_CRITICAL("invalid pbtxt config: {}", pbtxt);
                        return Status::CANCELLED;
                }

                dni::Graph* g = new dni::Graph(gc.value());
                std::string out = "max";

                SPDLOG_DEBUG("Create ObserveOutputStream: {}", out);
                g->ObserveOutputStream(out);

                g->PrepareForRun();

                std::vector<std::vector<double_t>> scores_vec = {
                    {0.8, 0.1, 0.4, 0.15},
                    {0.1, 0.2, 0.05, 0.7},
                    {0.7, 123.321, 567.1, 789.2}};
                for (auto&& scores : scores_vec)
                {
                        for (int i = 0; i < scores.size(); i++)
                        {
                                g->AddDatumToInputStream(
                                    "score_" + std::to_string(i + 1),
                                    dni::Datum(scores[i]));
                        }

                        g->RunOnce();

                        g->Wait();

                        auto ret = g->GetResult<double_t>(out);
                        SPDLOG_DEBUG("Gout {} result is: {}", out, ret);

                        std::unordered_map<std::string, std::string> g_outs;
                        g_outs[out] = std::to_string(ret);

                        GraphResponse response;
                        GraphCalcResult* result;
                        result = response.add_results();
                        buildGraphResult(g_outs, result);
                        writer->Write(response);

                        g->ClearResult();

                        std::this_thread::sleep_for(std::chrono::milliseconds(1234));
                }
        }
        return Status::OK;
}

void DNIServiceImpl::buildGraphResult(
    const std::unordered_map<std::string, std::string>& g_outs, GraphCalcResult* result)
{
        result->set_ts(Utils::now());
        result->set_type(GraphResultType::GraphLoadOK);

        google::protobuf::Map<std::string, std::string>* pMap = result->mutable_results();

        for (auto&& out : g_outs)
        {
                (*pMap)[out.first] = out.second;
        }
}

void DNIServiceImpl::printHeaders(const ServerContext* context)
{
        const multimap<grpc::string_ref, grpc::string_ref>& metadata =
            context->client_metadata();
        for (const auto& iter : metadata)
        {
                const grpc::string_ref& key = iter.first;
                const grpc::string_ref& value = iter.second;
                SPDLOG_DEBUG(
                    "->H {}: {}", std::string(key.begin(), key.end()),
                    std::string(value.begin(), value.end()));
        }
}

void DNIServiceImpl::propagateHeaders(
    const ServerContext* context, grpc::ClientContext& c)
{
        const multimap<grpc::string_ref, grpc::string_ref>& metadata =
            context->client_metadata();
        for (const auto& iter : metadata)
        {
                const grpc::string_ref& key = iter.first;
                const grpc::string_ref& value = iter.second;
                SPDLOG_DEBUG(
                    "->H {}: {}", std::string(key.begin(), key.end()),
                    std::string(value.begin(), value.end()));
        }
}

void DNIServiceImpl::setChannel(const std::shared_ptr<Channel>& channel)
{
        if (channel != nullptr)
        {
                client = DNIService::NewStub(channel);
        }
}
