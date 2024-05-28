
#include <memory>
#include <mutex>
#include <string>

#include "grpcpp/ext/proto_server_reflection_plugin.h"
#include "grpcpp/grpcpp.h"
#include "grpcpp/health_check_service_interface.h"
#include "samples/dni_grpc/common/connection.h"
#include "samples/dni_grpc/common/utils.h"
#include "samples/dni_grpc/protos/dni_service.grpc.pb.h"

using dni::Connection;
using dni::Utils;
using dni::service::grpc::DNIService;
using dni::service::grpc::GraphCalcResult;
using dni::service::grpc::GraphRequest;
using dni::service::grpc::GraphResponse;
using dni::service::grpc::GraphResultType;
using google::protobuf::Map;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;

class DNIServiceImpl final: public DNIService::Service {
public:
        Status CalculateGraph(
            ServerContext* context, const GraphRequest* request,
            ServerWriter<GraphResponse>* writer) override;

        void buildGraphResult(
            const std::unordered_map<std::string, std::string>& g_outs,
            GraphCalcResult* result);

        static void printHeaders(const ServerContext* context);

        static void propagateHeaders(
            const ServerContext* context, grpc::ClientContext& c);

        void setChannel(const std::shared_ptr<Channel>& channel);

private:
        std::unique_ptr<DNIService::Stub> client;

        std::mutex mutex_;
};
