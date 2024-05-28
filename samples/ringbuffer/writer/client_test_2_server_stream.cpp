#include "samples/grpc/client/proto_client.h"
#include "spdlog/spdlog.h"

int main(__attribute__((unused)) int argc, char** argv)
{
        spdlog::set_level(spdlog::level::trace);

        LandingClient landingClient(Connection::getChannel());

        std::string fpath = "samples/ringbuffer/writer/testdata/graph3.pbtxt";

        SPDLOG_DEBUG("Server streaming RPC");
        landingClient.TalkOneAnswerMore(fpath);

        return 0;
}