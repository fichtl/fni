#include "dni_client.h"
#include "spdlog/spdlog.h"

int main(__attribute__((unused)) int argc, char** argv)
{
        spdlog::set_level(spdlog::level::trace);

        DNIClient dni_clt(Connection::getChannel());

        std::string fpath = "samples/dni_grpc/client/testdata/max.pbtxt";

        SPDLOG_DEBUG("Server streaming RPC");
        dni_clt.CalculateGraph(fpath);

        return 0;
}