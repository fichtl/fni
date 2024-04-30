#include <netinet/ether.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <pcap.h>

#include "dni/framework/framework.h"
#include "spdlog/spdlog.h"

void load_packet_handler(
    u_char* args, const struct pcap_pkthdr* packet_header, const u_char* packet_body)
{
        auto packets = (std::vector<std::unordered_map<std::string, uint32_t>>*) args;
        std::unordered_map<std::string, uint32_t> packet;

        SPDLOG_DEBUG("Packet capture length: {}", packet_header->caplen);
        SPDLOG_DEBUG("Packet total length {}", packet_header->len);

        packet["Length"] = packet_header->len;

        struct ether_header* eth_header = (struct ether_header*) packet_body;

        if (ntohs(eth_header->ether_type) == ETHERTYPE_IP)
        {
                struct iphdr* ip_header =
                    (struct iphdr*) (packet_body + sizeof(struct ether_header));

                char ipv4[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &ip_header->saddr, ipv4, sizeof(ipv4));
                SPDLOG_DEBUG("src ip addr is: {}", ipv4);
                packet["SIP"] = ntohl(ip_header->saddr);

                inet_ntop(AF_INET, &ip_header->daddr, ipv4, sizeof(ipv4));
                SPDLOG_DEBUG("dst ip addr is: {}", ipv4);
                packet["DIP"] = ntohl(ip_header->daddr);

                SPDLOG_DEBUG("protocol is : {}", ip_header->protocol);
                switch (ip_header->protocol)
                {
                case IPPROTO_TCP: {
                        struct tcphdr* tcp =
                            (struct tcphdr*) ((u_char*) ip_header + ip_header->ihl * 4);
                        SPDLOG_DEBUG("src_port = {}", ntohs(tcp->source));
                        packet["SPort"] = ntohs(tcp->source);

                        SPDLOG_DEBUG("dst_port = {}", ntohs(tcp->dest));
                        packet["DPort"] = ntohs(tcp->dest);

                        SPDLOG_DEBUG("-------------------------------\n\n");
                        packet["Protocol"] = 6;

                        break;
                }

                case IPPROTO_UDP: {
                        struct udphdr* udp =
                            (struct udphdr*) ((u_char*) ip_header + ip_header->ihl * 4);
                        SPDLOG_DEBUG("src_port = {}", ntohs(udp->source));
                        packet["SPort"] = ntohs(udp->source);

                        SPDLOG_DEBUG("dst_port = {}", ntohs(udp->dest));
                        packet["DPort"] = ntohs(udp->dest);

                        SPDLOG_DEBUG("-------------------------------\n\n");
                        packet["Protocol"] = 17;

                        break;
                }

                case IPPROTO_ICMP: {
                        packet["Protocol"] = 1;
                        return;
                }

                default: return;
                }
        }

        packets->push_back(packet);

        return;
}

namespace dni {

        class PcapParseTask: public TaskBase {
        public:
                PcapParseTask(): name_("PcapParseTask") {}
                ~PcapParseTask() override {}

                int Open(TaskContext* ctx) override
                {
                        SPDLOG_DEBUG("Task {}: open task ...", name_);

                        return 0;
                }

                int Process(TaskContext* ctx) override
                {
                        // input
                        Datum pcap_d = ctx->Inputs()[0].Value();
                        SPDLOG_DEBUG("Task {}: Consume Datum: {}", name_, pcap_d);
                        auto pcap_opt = pcap_d.Consume<std::string>();
                        if (!pcap_opt)
                        {
                                SPDLOG_WARN(
                                    "Task {}: Consume() returns NULL, wait for "
                                    "input ...",
                                    name_);
                        }
                        auto pcap = *(pcap_opt.value());
                        SPDLOG_DEBUG("Task {}: pcap: {}", name_, pcap);

                        // parse
                        std::vector<std::unordered_map<std::string, uint32_t>> packets;
                        load_pcap_file(pcap, &packets);
                        SPDLOG_DEBUG("Task {}: packets size: {}", name_, packets.size());

                        ctx->Outputs()[0].AddDatum(Datum(packets));

                        return 0;
                }

                int Close(TaskContext* ctx) override
                {
                        SPDLOG_DEBUG("Task {}: closing ...", name_);

                        return 0;
                }

        private:
                int load_pcap_file(
                    std::string pcap_path,
                    std::vector<std::unordered_map<std::string, uint32_t>>* packets)
                {
                        SPDLOG_DEBUG(
                            "Task {}: loading pcap file {} ...", name_, pcap_path);
                        char errbuf[PCAP_ERRBUF_SIZE];

                        pcap_t* pcap_handle =
                            pcap_open_offline(pcap_path.c_str(), errbuf);
                        if (pcap_handle == NULL)
                        {
                                SPDLOG_ERROR(
                                    "fail to execute pcap_open_offline({}), error is {}",
                                    pcap_path,
                                    errbuf);
                                return -1;
                        }
                        pcap_activate(pcap_handle);
                        pcap_loop(
                            pcap_handle, -1, load_packet_handler, (u_char*) packets);

                        return 0;
                }

                std::string name_;
        };

        REGISTER(PcapParseTask);

}   // namespace dni
