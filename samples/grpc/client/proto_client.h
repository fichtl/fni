#pragma once

#include <memory>
#include <string>

#include "grpcpp/grpcpp.h"
#include "samples/grpc/common/connection.h"
#include "samples/grpc/common/utils.h"
#include "samples/grpc/protos/landing.grpc.pb.h"

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

class LandingClient {
public:
        explicit LandingClient(const std::shared_ptr<Channel>& channel)
            : client(LandingService::NewStub(channel))
        {}

        void Talk(const std::string& fpath);

        void TalkOneAnswerMore(const std::string& fpath);

        void TalkMoreAnswerOne(const std::string& fpath);

        void TalkBidirectional(const std::string& fpath1, const std::string& fpath2);

        static void printGraphResponse(
            ClientContext& context, const TalkResponse& response);

        static void buildHeaders(ClientContext& context);

private:
        std::unique_ptr<LandingService::Stub> client;
};
