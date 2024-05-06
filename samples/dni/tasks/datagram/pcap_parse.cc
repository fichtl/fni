#include <chrono>
#include <thread>

#include "dni/framework/framework.h"
#include "spdlog/spdlog.h"

#define PCAP_DUMP_FILE_PATH "samples/dni/tasks/datagram/testdata/pcap_parse.pcap"

void inject_after(dni::Graph* g, int after, int n, int interval)
{
        std::this_thread::sleep_for(std::chrono::milliseconds(after));

        for (int i = 0; i < n; i++)
        {
                SPDLOG_DEBUG(
                    "send Datum({}) to graph g({:p})",
                    dni::Datum(std::string(PCAP_DUMP_FILE_PATH)), fmt::ptr(g));
                g->AddDatumToInputStream(
                    "pcap", dni::Datum(std::string(PCAP_DUMP_FILE_PATH)));

                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        }
}

int main()
{
        spdlog::set_level(spdlog::level::trace);

        const std::string& proto = R"pb(
                type: "PcapParse"

                input_stream: "GIn:0:pcap"
                output_stream: "GOut:0:parsed_packets"

                node {
                  name: "A"
                  task: "PcapParseTask"
                  input_stream: "GIn:0:pcap"
                  output_stream: "GOut:0:parsed_packets"
                }
        )pb";

        auto gc = dni::ParseStringToGraphConfig(proto);
        if (!gc)
        {
                spdlog::error("invalid pbtxt config: {}", proto);
                return -1;
        }
        spdlog::debug(
            "GraphConfig: {}, node size: {}", gc.value().type(), gc.value().node_size());

        dni::Graph* g = new dni::Graph(gc.value());

        std::string out = "parsed_packets";

        spdlog::debug("Create ObserveOutputStream: {}", out);
        g->ObserveOutputStream(out);

        g->PrepareForRun();

        inject_after(g, 0, 1, 0);

        g->RunOnce();

        g->Wait();
        

        auto ret = std::move(
            g->GetResult<std::vector<std::unordered_map<std::string, uint32_t>>>(out));
        spdlog::info("Gout {} result is: {}", out, ret);

        g->Finish();

        spdlog::info("main over");

        return 0;
}
