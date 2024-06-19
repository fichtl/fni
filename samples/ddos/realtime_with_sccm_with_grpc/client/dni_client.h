#pragma once

#include <memory>
#include <string>

#include "grpcpp/grpcpp.h"
#include "samples/ddos/realtime_with_sccm_with_grpc/common/connection.h"
#include "samples/ddos/realtime_with_sccm_with_grpc/common/utils.h"
#include "samples/ddos/realtime_with_sccm_with_grpc/protos/dni_service.grpc.pb.h"

using dni::Connection;
using dni::Utils;
using dni::service::grpc::DNIService;
using dni::service::grpc::GraphCalcResult;
using dni::service::grpc::GraphDMSRule;
using dni::service::grpc::GraphRequest;
using dni::service::grpc::GraphResponse;
using dni::service::grpc::GraphResultType;
using google::protobuf::Map;
using google::protobuf::RepeatedPtrField;
using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;

class DNIClient {
public:
        explicit DNIClient(const std::shared_ptr<Channel>& channel)
            : client(DNIService::NewStub(channel))
        {}

        void CalculateGraph(const std::string& fpath);

        static void printGraphResponse(
            ClientContext& context, const GraphResponse& response);

        static void buildHeaders(ClientContext& context);

private:
        std::unique_ptr<DNIService::Stub> client;
};
