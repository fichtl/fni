#include "proto_server.h"
#include "spdlog/spdlog.h"

// https://myssl.com/create_test_cert.html
__attribute__((unused)) const char cert[] = "/var/hello_grpc/server_certs/cert.pem";
const char certKey[] = "/var/hello_grpc/server_certs/private.key";
const char certChain[] = "/var/hello_grpc/server_certs/full_chain.pem";
const char rootCert[] = "/var/hello_grpc/server_certs/myssl_root.cer";

void RunServer()
{
        const std::string& port = Utils::getServerPort();
        // std::string server_address("172.17.17.86:" + port);
        std::string server_address("127.0.0.1:" + port);

        grpc::EnableDefaultHealthCheckService(true);
        grpc::reflection::InitProtoReflectionServerBuilderPlugin();
        ServerBuilder builder;
        const std::string& secure = Utils::getSecure();
        if (!secure.empty() && secure == "Y")
        {
                SPDLOG_DEBUG("Start GRPC TLS Server, port: {}", port);

                grpc::SslServerCredentialsOptions ssl_opts(
                    GRPC_SSL_REQUEST_CLIENT_CERTIFICATE_BUT_DONT_VERIFY);
                ssl_opts.pem_root_certs = Connection::getFileContent(rootCert);
                grpc::SslServerCredentialsOptions::PemKeyCertPair pemKeyCertPair;
                pemKeyCertPair.private_key = Connection::getFileContent(certKey);
                pemKeyCertPair.cert_chain = Connection::getFileContent(certChain);
                ssl_opts.pem_key_cert_pairs.push_back({pemKeyCertPair});
                builder.AddListeningPort(
                    server_address, grpc::SslServerCredentials(ssl_opts));
        }
        else
        {
                SPDLOG_DEBUG("Start GRPC Server, port: {}", port);
                builder.AddListeningPort(
                    server_address, grpc::InsecureServerCredentials());
        }

        LandingServiceImpl landingService;
        const char* backend = getenv("GRPC_HELLO_BACKEND");
        std::string endpoint(backend ? backend : "");
        if (!endpoint.empty())
        {
                landingService.setChannel(Connection::getChannel());
        }
        builder.RegisterService(&landingService);
        std::unique_ptr<Server> server(builder.BuildAndStart());
        SPDLOG_DEBUG("Server listening on {}", server_address);

        server->Wait();
}

int main(__attribute__((unused)) int argc, char** argv)
{
        spdlog::set_level(spdlog::level::trace);

        RunServer();
        SPDLOG_INFO("Hello gRPC C++ Server is stopping");

        return 0;
}
