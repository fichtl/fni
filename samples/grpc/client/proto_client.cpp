#include "proto_client.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>
#include <vector>

#include "spdlog/spdlog.h"

using dni::Connection;
using dni::Utils;
using dni::samples::grpc::LandingService;
using dni::samples::grpc::ResultType;
using dni::samples::grpc::TalkRequest;
using dni::samples::grpc::TalkResponse;
using dni::samples::grpc::TalkResult;
using google::protobuf::Map;
using google::protobuf::RepeatedPtrField;
using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;

void LandingClient::Talk(const std::string& fpath)
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
        TalkResponse talkResponse;
        TalkRequest talkRequest;
        talkRequest.set_data(ss.str());
        talkRequest.set_meta("C++");
        Status status = client->Talk(&context, talkRequest, &talkResponse);
        if (status.ok())
        {
                printGraphResponse(context, talkResponse);
        }
        else
        {
                SPDLOG_ERROR(
                    "Error: {}: {}", (int) status.error_code(), status.error_message());
        }
}

void LandingClient::TalkOneAnswerMore(const std::string& fpath)
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
        TalkResponse talkResponse;
        TalkRequest talkRequest;
        talkRequest.set_data(ss.str());
        talkRequest.set_meta("C++");
        const std::unique_ptr<::grpc::ClientReader<TalkResponse>>& response(
            client->TalkOneAnswerMore(&context, talkRequest));
        while (response->Read(&talkResponse))
        {
                SPDLOG_DEBUG("Receive one result from dni ...");
                printGraphResponse(context, talkResponse);
        }
}

void LandingClient::TalkMoreAnswerOne(const std::string& fpath)
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
        TalkResponse talkResponse;
        std::unique_ptr<ClientWriter<TalkRequest>> writer(
            client->TalkMoreAnswerOne(&context, &talkResponse));

        std::vector<std::string> pbtxts = {ss.str()};
        const std::list<TalkRequest>& pb_requests = Utils::buildLinkRequests(pbtxts);
        for (auto&& request : pb_requests)
        {
                if (!writer->Write(request))
                {
                        // Broken stream.
                        break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(5 * 1000));
        }

        SPDLOG_INFO("Guess server graph load ok...");

        std::vector<double_t> data = {0.8, 0.1, 0.4, 0.15};
        const std::list<TalkRequest>& data_requests = Utils::buildLinkDataRequests(data);
        for (auto&& request : data_requests)
        {
                if (!writer->Write(request))
                {
                        // Broken stream.
                        break;
                }
        }

        writer->WritesDone();
        Status status = writer->Finish();
        if (status.ok())
        {
                printGraphResponse(context, talkResponse);
        }
        else
        {
                SPDLOG_ERROR(
                    "Error: {}: {}", (int) status.error_code(), status.error_message());
        }
}

void LandingClient::TalkBidirectional(
    const std::string& fpath1, const std::string& fpath2)
{
        std::ifstream fin1(fpath1);
        if (!fin1.is_open())
        {
                SPDLOG_CRITICAL("can't open {}", fpath1);
                return;
        }
        std::stringstream ss1;
        ss1 << fin1.rdbuf();
        auto pbtxt1 = ss1.str();

        std::ifstream fin2(fpath2);
        if (!fin2.is_open())
        {
                SPDLOG_CRITICAL("can't open {}", fpath2);
                return;
        }
        std::stringstream ss2;
        ss2 << fin2.rdbuf();
        auto pbtxt2 = ss2.str();

        ClientContext context;
        buildHeaders(context);
        TalkResponse talkResponse;
        std::shared_ptr<ClientReaderWriter<TalkRequest, TalkResponse>> stream(
            client->TalkBidirectional(&context));

        std::thread writer([stream, pbtxt1, pbtxt2]() {
                std::vector<std::string> pbtxts = {pbtxt1, pbtxt2};
                const list<TalkRequest>& requests = Utils::buildLinkRequests(pbtxts);
                for (auto&& request : requests)
                {
                        SPDLOG_INFO("@@@@@@@@@@{}", request.data());
                        stream->Write(request);

                        std::this_thread::sleep_for(std::chrono::seconds(2));
                }
                stream->WritesDone();
        });

        while (stream->Read(&talkResponse))
        {
                printGraphResponse(context, talkResponse);
        }

        writer.join();
        Status status = stream->Finish();
        if (!status.ok())
        {
                SPDLOG_ERROR(
                    "Error: {}: {}", (int) status.error_code(), status.error_message());
        }
}

void LandingClient::printGraphResponse(
    ClientContext& context, const TalkResponse& response)
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
        const RepeatedPtrField<TalkResult>& talkResults = response.results();
        for (const TalkResult& result : talkResults)
        {
                const Map<std::string, std::string>& kv = result.kv();
                std::string id(kv.at("id"));
                std::string idx(kv.at("idx"));
                std::string meta(kv.at("meta"));
                std::string data(kv.at("data"));
                // SPDLOG_DEBUG(
                //     "{} {} [ {} {} {} {} {} ]",
                //     response.status(),
                //     result.id(),
                //     meta,
                //     ResultType_Name(result.type()),
                //     id,
                //     idx,
                //     data);

                SPDLOG_DEBUG("graph result: {}", data);
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

void LandingClient::buildHeaders(ClientContext& context)
{
        // examples/cpp/metadata
        context.AddMetadata("k1", "v1");
        context.AddMetadata("k2", "v2");
}
