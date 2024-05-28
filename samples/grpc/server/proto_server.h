
#include <memory>
#include <mutex>
#include <string>

#include "grpcpp/ext/proto_server_reflection_plugin.h"
#include "grpcpp/grpcpp.h"
#include "grpcpp/health_check_service_interface.h"
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
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;

class LandingServiceImpl final: public LandingService::Service {
public:
        Status Talk(
            ServerContext* context, const TalkRequest* request,
            TalkResponse* response) override;

        Status TalkOneAnswerMore(
            ServerContext* context, const TalkRequest* request,
            ServerWriter<TalkResponse>* writer) override;

        Status TalkMoreAnswerOne(
            ServerContext* context, ServerReader<TalkRequest>* reader,
            TalkResponse* response) override;

        Status TalkBidirectional(
            ServerContext* context,
            ServerReaderWriter<TalkResponse, TalkRequest>* stream) override;

        void f_pbtxt0(
            const std::string& pbtxt,
            ServerReaderWriter<TalkResponse, TalkRequest>*& stream);

        void f_pbtxt1(
            const std::string& pbtxt,
            ServerReaderWriter<TalkResponse, TalkRequest>*& stream);

        void buildGraphResult(
            const std::string& pbtxt, const std::string& ret, TalkResult* talkResult);

        static void printHeaders(const ServerContext* context);

        static void propagateHeaders(
            const ServerContext* context, grpc::ClientContext& c);

        void setChannel(const std::shared_ptr<Channel>& channel);

private:
        std::unique_ptr<LandingService::Stub> client;

        std::mutex mutex_;
};
