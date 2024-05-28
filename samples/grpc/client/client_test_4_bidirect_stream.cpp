#include "proto_client.h"
#include "spdlog/spdlog.h"

int main(__attribute__((unused)) int argc, char** argv)
{
        spdlog::set_level(spdlog::level::trace);
        SPDLOG_DEBUG("Bidirectional streaming RPC");

        LandingClient landingClient(Connection::getChannel());
        std::string fpath = "samples/grpc/client/testdata/max.pbtxt";
        std::string fpath2 = "samples/grpc/client/testdata/sum.pbtxt";
        landingClient.TalkBidirectional(fpath, fpath2);

        return 0;
}