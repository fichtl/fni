#include "utils.h"

#include <chrono>
#include <iostream>
#include <list>
#include <map>
#include <random>
#include <string>

#include "spdlog/spdlog.h"

using std::getenv;

namespace dni {

        std::string Utils::uuid()
        {
                /*
                absl::BitGen bit_gen;
                #include "absl/random/distributions.h"
                #include "absl/random/random.h"
                #include "absl/strings/str_cat.h"
                absl::uniform_int_distribution<uint32_t> distribution;
                uint32_t random_uuid = distribution(bit_gen);
                std::string uuid = absl::StrCat(absl::FormatTime("%Y-%m-%d-%H-%M-%S-",
                absl::Now(), absl::LocalTimeZone()), random_uuid);
                */

                std::random_device rd;
                std::mt19937 gen(rd());
                unsigned char bytes[16];
                std::generate(std::begin(bytes), std::end(bytes), std::ref(gen));
                std::string uuid_str;
                uuid_str += std::to_string((bytes[6] & 0x0F) << 4 | (bytes[7] & 0x0F));
                uuid_str += "-";
                uuid_str += std::to_string((bytes[8] & 0x3F) << 4 | (bytes[9] & 0x0F));
                uuid_str += "-";
                uuid_str += std::to_string((bytes[10] & 0x3F) << 4 | (bytes[11] & 0x0F));
                uuid_str += "-";
                uuid_str += std::to_string((bytes[12] & 0x3F) << 4 | (bytes[13] & 0x0F));
                uuid_str += "-";
                uuid_str += std::to_string(bytes[14] >> 4);
                uuid_str += std::to_string(bytes[14] & 0x0F);
                return uuid_str;
        }

        long Utils::now()
        {
                const auto now = chrono::system_clock::now();
                auto value = now.time_since_epoch().count();
                return value;
        }

        // std::string Utils::getServerHost()
        // {
        //         const char* server_address = "172.17.17.86";
        //         std::string endpoint(server_address ? server_address : "localhost");
        //         return endpoint;
        // }

        string Utils::getServerHost()
        {
                const char* server_address = getenv("GRPC_SERVER");
                string endpoint(server_address ? server_address : "localhost");
                return endpoint;
        }

        std::string Utils::getBackend()
        {
                const char* server_address = getenv("GRPC_DNI_SERVER_BACKEND");
                std::string endpoint(server_address ? server_address : "");
                if (endpoint.empty())
                {
                        return getServerHost();
                }
                return endpoint;
        }

        std::string Utils::getServerPort()
        {
                const char* serverPort = getenv("GRPC_SERVER_PORT");
                std::string port(serverPort ? serverPort : "9527");
                return port;
        }

        std::string Utils::getBackendPort()
        {
                const char* port = getenv("GRPC_DNI_SERVER_BACKEND_PORT");
                std::string backendPort(port ? port : "");
                if (backendPort.empty())
                {
                        return getServerPort();
                }
                return backendPort;
        }

        std::string Utils::getSecure()
        {
                const char* isTls = getenv("GRPC_DNI_SERVER_SECURE");
                std::string secure(isTls ? isTls : "");
                return secure;
        }

}   // namespace dni