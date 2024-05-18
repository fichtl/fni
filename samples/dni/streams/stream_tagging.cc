#include <chrono>
#include <thread>

#include "dni/framework/framework.h"
#include "spdlog/spdlog.h"

void inject_after(dni::Graph* g, dni::Datum&& d, int after, int n, int interval)
{
        std::this_thread::sleep_for(std::chrono::milliseconds(after));

        for (int i = 0; i < n; i++)
        {
                SPDLOG_DEBUG("send Datum({}) to graph g({:p})", d, fmt::ptr(g));

                g->AddDatumToInputStream("GIn0", d);
                g->AddDatumToInputStream("GIn1", d);
                g->AddDatumToInputStream("GIn2", d);

                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        }
}

int main()
{
        spdlog::set_level(spdlog::level::trace);

        const std::string& proto = R"pb(
                type: "Counter"

                input_stream: "PCAP:0:GIn0"
                input_stream: "STAT:1:GIn2"
                input_stream: "STAT:0:GIn1"
                output_stream: "SCORE:0:GOutSSS"

                node {
                  name: "A"
                  task: "TransparentTask"
                  input_stream: "PCAP:GIn0"
                  input_stream: "STAT:GIn1"
                  output_stream: "SCORE:AOut0"
                  options {
                        [type.asnapis.io/dni.TransparentTaskOptions] {
                                observe: { tag: "PCAP" index: 0 }
                                observe: { tag: "PCAP" index: 1 }
                                observe: { tag: "STAT" index: 0 }
                                observe: { tag: "STAT" index: 1 }
                                observe: { tag: "SCORE" index: 0 }
                                observe: { tag: "SCORE" index: 1 }
                        }
                  }
                }

                node {
                  name: "B"
                  task: "TransparentTask"
                  input_stream: "SCORE:AOut0"
                  input_stream: "STAT:GIn2"
                  output_stream: "BOut"
                  options {
                        [type.asnapis.io/dni.TransparentTaskOptions] {
                                observe: { tag: "" index: 0 }
                                observe: { tag: "" index: 1 }
                                observe: { tag: "" index: 2 }
                                observe: { tag: "" index: 3 }
                                observe: { tag: "PCAP" index: 0 }
                                observe: { tag: "PCAP" index: 1 }
                                observe: { tag: "STAT" index: 0 }
                                observe: { tag: "STAT" index: 1 }
                                observe: { tag: "SCORE" index: 0 }
                                observe: { tag: "SCORE" index: 1 }
                        }
                  }
                }

                node {
                  name: "C"
                  task: "TransparentTask"
                  input_stream: "SCORE:AOut"
                  input_stream: "SCORE:1:BOut"
                  output_stream: "SCORE:0:COut"
                  options {
                        [type.asnapis.io/dni.TransparentTaskOptions] {
                                observe: { tag: "PCAP" index: 0 }
                                observe: { tag: "PCAP" index: 1 }
                                observe: { tag: "STAT" index: 0 }
                                observe: { tag: "STAT" index: 1 }
                                observe: { tag: "SCORE" index: 0 }
                                observe: { tag: "SCORE" index: 1 }
                        }
                  }
                }

                node {
                  name: "D"
                  task: "TransparentTask"
                  input_stream: "SCORE:COut"
                  output_stream: "SCORE:0:GOutSSS"
                  options {
                        [type.asnapis.io/dni.TransparentTaskOptions] {
                                observe: { tag: "PCAP" index: 0 }
                                observe: { tag: "PCAP" index: 1 }
                                observe: { tag: "STAT" index: 0 }
                                observe: { tag: "STAT" index: 1 }
                                observe: { tag: "SCORE" index: 0 }
                                observe: { tag: "SCORE" index: 1 }
                        }
                  }
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

        std::string out = "GOutSSS";

        spdlog::debug("Create ObserveOutputStream: {}", out);
        g->ObserveOutputStream(out);

        g->PrepareForRun();

        inject_after(g, dni::Datum(10), 0, 1, 0);

        g->RunOnce();

        g->Wait();

        int ret = g->GetResult<int>(out);
        spdlog::info("Gout result is: {:d}", ret);

        g->Finish();

        spdlog::info("main over");

        return 0;
}
