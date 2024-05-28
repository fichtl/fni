#include "proto_server.h"

#include <chrono>
#include <thread>

#include "dni/framework/framework.h"
#include "spdlog/spdlog.h"

Status LandingServiceImpl::Talk(
    ServerContext* context, const TalkRequest* request, TalkResponse* response)
{
        if (client != nullptr)
        {
                grpc::ClientContext c;
                propagateHeaders(context, c);
                return client->Talk(&c, *request, response);
        }
        else
        {
                printHeaders(context);
                context->AddInitialMetadata("h1", "v1");
                context->AddTrailingMetadata("l1", "v1");
                const std::string& pbtxt = request->data();   // pbtxt

                SPDLOG_DEBUG("TALK REQUEST: data={}, meta={}", pbtxt, request->meta());

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

                std::vector<double_t> scores = {0.8, 0.1, 0.4, 0.15};

                for (int i = 0; i < scores.size(); i++)
                {
                        g->AddDatumToInputStream(
                            "score_" + std::to_string(i + 1), dni::Datum(scores[i]));
                }

                g->RunOnce();

                g->Wait();

                auto ret = g->GetResult<double_t>(out);
                SPDLOG_DEBUG("Gout {} result is: {}", out, ret);

                g->Finish();

                SPDLOG_DEBUG("graph running over");

                response->set_status(200);
                TalkResult* talkResult;
                talkResult = response->add_results();
                buildGraphResult(pbtxt, std::to_string(ret), talkResult);
                return Status::OK;
        }
}

Status LandingServiceImpl::TalkOneAnswerMore(
    ServerContext* context, const TalkRequest* request,
    ServerWriter<TalkResponse>* writer)
{
        printHeaders(context);
        if (client != nullptr)
        {
                grpc::ClientContext c;
                propagateHeaders(context, c);
                TalkResponse talkResponse;
                const std::unique_ptr<::grpc::ClientReader<TalkResponse>>& response(
                    client->TalkOneAnswerMore(&c, *request));
                while (response->Read(&talkResponse))
                {
                        writer->Write(talkResponse);
                }
        }
        else
        {
                const std::string& pbtxt = request->data();
                SPDLOG_DEBUG(
                    "TalkOneAnswerMore REQUEST: data={}, meta={}", pbtxt,
                    request->meta());

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

                        g->ClearResult();

                        TalkResponse response;
                        response.set_status(200);
                        TalkResult* talkResult;
                        talkResult = response.add_results();
                        buildGraphResult(pbtxt, std::to_string(ret), talkResult);
                        writer->Write(response);

                        std::this_thread::sleep_for(std::chrono::milliseconds(1234));
                }
        }
        return Status::OK;
}

Status LandingServiceImpl::TalkMoreAnswerOne(
    ServerContext* context, ServerReader<TalkRequest>* reader, TalkResponse* response)
{
        printHeaders(context);
        if (client != nullptr)
        {
                grpc::ClientContext c;
                propagateHeaders(context, c);
                std::unique_ptr<grpc::ClientWriter<TalkRequest>> writer(
                    client->TalkMoreAnswerOne(&c, response));
                TalkRequest request;
                while (reader->Read(&request))
                {
                        if (!writer->Write(request))
                        {
                                // Broken stream.
                                break;
                        }
                }
                writer->WritesDone();
                return writer->Finish();
        }
        else
        {
                TalkRequest request;
                dni::Graph* g;
                std::string out;

                while (reader->Read(&request))
                {
                        const std::string& pbtxt = request.data();
                        SPDLOG_DEBUG(
                            "TalkOneAnswerMore REQUEST: data={}, meta={}", pbtxt,
                            request.meta());

                        auto gc = dni::ParseStringToGraphConfig(pbtxt);
                        if (!gc)
                        {
                                SPDLOG_CRITICAL("invalid pbtxt config: {}", pbtxt);
                                return Status::CANCELLED;
                        }

                        g = new dni::Graph(gc.value());
                        out = "max";

                        SPDLOG_DEBUG("Create ObserveOutputStream: {}", out);
                        g->ObserveOutputStream(out);

                        g->PrepareForRun();

                        break;
                }

                int i = 0;
                while (reader->Read(&request))
                {
                        const std::string& d = request.data();
                        SPDLOG_DEBUG(
                            "TalkMoreAnswerOne REQUEST: data={}, meta={}", d,
                            request.meta());

                        g->AddDatumToInputStream(
                            "score_" + std::to_string(i + 1), dni::Datum(std::stod(d)));

                        i++;
                }

                if (i != 4)
                {
                        response->set_status(-1);
                        return Status::CANCELLED;
                }

                g->RunOnce();

                g->Wait();

                auto ret = g->GetResult<double_t>(out);
                SPDLOG_DEBUG("Gout {} result is: {}", out, ret);

                g->ClearResult();

                response->set_status(200);
                TalkResult* talkResult;
                talkResult = response->add_results();
                buildGraphResult("max-task", std::to_string(ret), talkResult);

                return Status::OK;
        }
}

Status LandingServiceImpl::TalkBidirectional(
    ServerContext* context, ServerReaderWriter<TalkResponse, TalkRequest>* stream)
{
        printHeaders(context);
        if (client != nullptr)
        {
                grpc::ClientContext c;
                propagateHeaders(context, c);
                TalkResponse talkResponse;
                std::shared_ptr<grpc::ClientReaderWriter<TalkRequest, TalkResponse>> s(
                    client->TalkBidirectional(&c));
                TalkRequest request;
                while (stream->Read(&request))
                {
                        s->Write(request);
                }
                while (s->Read(&talkResponse))
                {
                        stream->Write(talkResponse);
                }
                return s->Finish();
        }
        else
        {
                TalkRequest request;
                std::map<std::string, bool> running;
                int i = 0;
                std::thread threads[2];
                while (stream->Read(&request))
                {
                        SPDLOG_DEBUG(
                            "TalkBidirectional REQUEST: meta={}", request.meta());

                        const std::string& meta = request.meta();
                        if (running[meta])
                        {
                                SPDLOG_INFO("{} is already running", meta);
                                continue;
                        }

                        running[meta] = true;

                        if (meta == "pbtxt_0")
                        {
                                threads[i] = std::thread(
                                    [&]() { f_pbtxt0(request.data(), stream); });
                        }
                        else if (meta == "pbtxt_1")
                        {
                                threads[i] = std::thread(
                                    [&]() { f_pbtxt1(request.data(), stream); });
                        }

                        i++;

                        if (i == 2)
                        {
                                break;
                        }
                }

                SPDLOG_DEBUG("Done spawning threads! Now wait for them to join");
                for (auto& t : threads)
                {
                        t.join();
                }
                SPDLOG_DEBUG("All threads joined.");

                return Status::OK;
        }
}

void LandingServiceImpl::f_pbtxt0(
    const std::string& pbtxt, ServerReaderWriter<TalkResponse, TalkRequest>*& stream)
{
        auto gc = dni::ParseStringToGraphConfig(pbtxt);
        if (!gc)
        {
                SPDLOG_CRITICAL("invalid pbtxt config: {}", pbtxt);
                return;
        }

        dni::Graph* g = new dni::Graph(gc.value());
        std::string out = "max";

        SPDLOG_DEBUG("Create ObserveOutputStream: {}", out);
        g->ObserveOutputStream(out);

        g->PrepareForRun();

        std::vector<std::vector<double_t>> scores_vec = {
            {0.8, 0.1, 0.4, 0.15}, {0.1, 0.2, 0.05, 0.7}, {0.7, 123.321, 567.1, 789.2}};
        for (auto&& scores : scores_vec)
        {
                for (int i = 0; i < scores.size(); i++)
                {
                        g->AddDatumToInputStream(
                            "score_" + std::to_string(i + 1), dni::Datum(scores[i]));
                }

                g->RunOnce();

                g->Wait();

                auto ret = g->GetResult<double_t>(out);
                SPDLOG_DEBUG("Gout {} result is: {}", out, ret);

                g->ClearResult();

                TalkResponse response;
                response.set_status(200);
                TalkResult* talkResult;
                talkResult = response.add_results();
                buildGraphResult("pbtxt_0", std::to_string(ret), talkResult);

                mutex_.lock();
                stream->Write(response);
                mutex_.unlock();

                std::this_thread::sleep_for(std::chrono::milliseconds(1234));
        }
}

void LandingServiceImpl::f_pbtxt1(
    const std::string& pbtxt, ServerReaderWriter<TalkResponse, TalkRequest>*& stream)
{
        auto gc = dni::ParseStringToGraphConfig(pbtxt);
        if (!gc)
        {
                SPDLOG_CRITICAL("invalid pbtxt config: {}", pbtxt);
                return;
        }

        dni::Graph* g = new dni::Graph(gc.value());
        std::string out = "sum";

        SPDLOG_DEBUG("Create ObserveOutputStream: {}", out);
        g->ObserveOutputStream(out);

        g->PrepareForRun();

        std::vector<std::vector<double_t>> scores_vec = {
            {0.8, 0.1, 0.4, 0.15}, {0.1, 0.2, 0.05, 0.7}, {0.7, 3.3, 5.1, 7.2}};
        for (auto&& scores : scores_vec)
        {
                for (int i = 0; i < scores.size(); i++)
                {
                        g->AddDatumToInputStream(
                            "score_" + std::to_string(i + 1), dni::Datum(scores[i]));
                }

                g->RunOnce();

                g->Wait();

                auto ret = g->GetResult<double_t>(out);
                SPDLOG_DEBUG("Gout {} result is: {}", out, ret);

                g->ClearResult();

                TalkResponse response;
                response.set_status(200);
                TalkResult* talkResult;
                talkResult = response.add_results();
                buildGraphResult("pbtxt_1", std::to_string(ret), talkResult);

                mutex_.lock();
                stream->Write(response);
                mutex_.unlock();

                std::this_thread::sleep_for(std::chrono::milliseconds(1347));
        }
}

void LandingServiceImpl::buildGraphResult(
    const std::string& pbtxt, const std::string& ret, TalkResult* talkResult)
{
        talkResult->set_id(Utils::now());
        talkResult->set_type(ResultType::OK);

        google::protobuf::Map<std::string, std::string>* pMap = talkResult->mutable_kv();

        const std::string& uuid = Utils::uuid();
        (*pMap)["id"] = uuid;
        (*pMap)["idx"] = pbtxt;
        (*pMap)["meta"] = "C++";
        (*pMap)["data"] = ret;
}

void LandingServiceImpl::printHeaders(const ServerContext* context)
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

void LandingServiceImpl::propagateHeaders(
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
                // c.AddMetadata((basic_string<char> &&) key, (basic_string<char>
                // &&) value);
        }
}

void LandingServiceImpl::setChannel(const std::shared_ptr<Channel>& channel)
{
        if (channel != nullptr)
        {
                client = LandingService::NewStub(channel);
        }
}
