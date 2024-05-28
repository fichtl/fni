#include "proto_client.h"
#include "spdlog/spdlog.h"

int main(__attribute__((unused)) int argc, char** argv)
{
        spdlog::set_level(spdlog::level::trace);

        LandingClient landingClient(Connection::getChannel());

        std::string fpath = "samples/grpc/client/testdata/max.pbtxt";
        SPDLOG_DEBUG("Unary RPC");
        landingClient.Talk(fpath);

        return 0;
}