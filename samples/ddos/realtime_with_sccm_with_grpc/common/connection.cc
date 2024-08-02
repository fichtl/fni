#include "connection.h"

#include <fstream>
#include <string>

#include "spdlog/spdlog.h"
#include "utils.h"

using grpc::Channel;

namespace dni {
std::string Connection::getFileContent(const char* path)
{
        std::ifstream stream(path);
        std::string contents;
        contents.assign(
            (std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
        stream.close();
        return contents;
}

shared_ptr<Channel> Connection::getChannel()
{
        // https://myssl.com/create_test_cert.html
        const char cert[] = "/var/dni_grpc/client_certs/cert.pem";
        const char certKey[] = "/var/dni_grpc/client_certs/private.key";
        const char certChain[] = "/var/dni_grpc/client_certs/full_chain.pem";
        const char rootCert[] = "/var/dni_grpc/client_certs/myssl_root.cer";
        const std::string serverName = "dni.grpc.io";

        const std::string& port = Utils::getBackendPort();
        const basic_string<char, char_traits<char>, allocator<char>>& target =
            Utils::getBackend() + ":" + port;
        const std::string& secure = Utils::getSecure();
        if (!secure.empty() && secure == "Y")
        {
                grpc::SslCredentialsOptions ssl_opts;
                ssl_opts.pem_root_certs = Connection::getFileContent(certChain);
                ssl_opts.pem_private_key = Connection::getFileContent(certKey);
                ssl_opts.pem_cert_chain = Connection::getFileContent(certChain);
                grpc::ChannelArguments channel_args;
                channel_args.SetString("grpc.default_authority", serverName);
                SPDLOG_DEBUG("Connect with TLS, port: {}", port);

                return grpc::CreateCustomChannel(
                    target, grpc::SslCredentials(ssl_opts), channel_args);
        }
        else
        {
                SPDLOG_DEBUG("Connect with InSecure, port: {}", port);

                return grpc::CreateChannel(target, grpc::InsecureChannelCredentials());
        }
}
}   // namespace dni
