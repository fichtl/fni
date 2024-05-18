#ifdef __APPLE__
#        include <netinet/if_ether.h>
#else
#        include <netinet/ether.h>
#endif
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <string>
#include <unordered_map>
#include <vector>

#include "dni/framework/framework.h"
#include "pcap.h"
#include "spdlog/spdlog.h"

namespace dni {

void parse_snd_features(u_char* args, const struct pcap_pkthdr* hdr, const u_char* body)
{
        auto packets = (std::vector<std::unordered_map<std::string, uint32_t>>*) args;
        std::unordered_map<std::string, uint32_t> packet;

        SPDLOG_TRACE("Packet total len: {}, capture len: {}", hdr->len, hdr->caplen);

        packet["Length"] = hdr->len;

#ifdef __APPLE__

        struct ether_header* eth_header = (struct ether_header*) body;

        if (ntohs(eth_header->ether_type) == ETHERTYPE_IP)
        {
                struct ip* ip_header = (struct ip*) (body + sizeof(struct ether_header));

                char ipv4[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &ip_header->ip_src, ipv4, sizeof(ipv4));
                SPDLOG_TRACE("src ip addr is: {}", ipv4);
                packet["SIP"] = ntohl(ip_header->ip_src.s_addr);

                inet_ntop(AF_INET, &ip_header->ip_dst, ipv4, sizeof(ipv4));
                SPDLOG_TRACE("dst ip addr is: {}", ipv4);
                packet["DIP"] = ntohl(ip_header->ip_dst.s_addr);

                SPDLOG_TRACE("protocol is : {}", ip_header->ip_p);
                switch (ip_header->ip_p)
                {
                case IPPROTO_TCP: {
                        struct tcphdr* tcp =
                            (struct tcphdr*) ((u_char*) ip_header + ip_header->ip_hl * 4);
                        SPDLOG_TRACE("src_port = {}", ntohs(tcp->th_sport));
                        packet["SPort"] = ntohs(tcp->th_sport);

                        SPDLOG_TRACE("dst_port = {}", ntohs(tcp->th_dport));
                        packet["DPort"] = ntohs(tcp->th_dport);

                        SPDLOG_TRACE("-------------------------------\n\n");
                        packet["Protocol"] = 6;

                        break;
                }

                case IPPROTO_UDP: {
                        struct udphdr* udp =
                            (struct udphdr*) ((u_char*) ip_header + ip_header->ip_hl * 4);
                        SPDLOG_TRACE("src_port = {}", ntohs(udp->uh_sport));
                        packet["SPort"] = ntohs(udp->uh_sport);

                        SPDLOG_TRACE("dst_port = {}", ntohs(udp->uh_dport));
                        packet["DPort"] = ntohs(udp->uh_dport);

                        SPDLOG_TRACE("-------------------------------\n\n");
                        packet["Protocol"] = 17;

                        break;
                }

                case IPPROTO_ICMP: {
                        packet["Protocol"] = 1;
                        break;
                }

                default: return;
                }
        }

#else

        struct ether_header* eth_header = (struct ether_header*) body;

        if (ntohs(eth_header->ether_type) == ETHERTYPE_IP)
        {
                struct iphdr* ip_header =
                    (struct iphdr*) (body + sizeof(struct ether_header));

                char ipv4[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &ip_header->saddr, ipv4, sizeof(ipv4));
                SPDLOG_TRACE("src ip addr is: {}", ipv4);
                packet["SIP"] = ntohl(ip_header->saddr);

                inet_ntop(AF_INET, &ip_header->daddr, ipv4, sizeof(ipv4));
                SPDLOG_TRACE("dst ip addr is: {}", ipv4);
                packet["DIP"] = ntohl(ip_header->daddr);

                SPDLOG_TRACE("protocol is : {}", ip_header->protocol);
                switch (ip_header->protocol)
                {
                case IPPROTO_TCP: {
                        struct tcphdr* tcp =
                            (struct tcphdr*) ((u_char*) ip_header + ip_header->ihl * 4);
                        SPDLOG_TRACE("src_port = {}", ntohs(tcp->source));
                        packet["SPort"] = ntohs(tcp->source);

                        SPDLOG_TRACE("dst_port = {}", ntohs(tcp->dest));
                        packet["DPort"] = ntohs(tcp->dest);

                        SPDLOG_TRACE("-------------------------------\n\n");
                        packet["Protocol"] = 6;

                        break;
                }

                case IPPROTO_UDP: {
                        struct udphdr* udp =
                            (struct udphdr*) ((u_char*) ip_header + ip_header->ihl * 4);
                        SPDLOG_TRACE("src_port = {}", ntohs(udp->source));
                        packet["SPort"] = ntohs(udp->source);

                        SPDLOG_TRACE("dst_port = {}", ntohs(udp->dest));
                        packet["DPort"] = ntohs(udp->dest);

                        SPDLOG_TRACE("-------------------------------\n\n");
                        packet["Protocol"] = 17;

                        break;
                }

                case IPPROTO_ICMP: {
                        packet["Protocol"] = 1;
                        break;
                }

                default: return;
                }
        }

#endif
        packets->push_back(packet);

        return;
}

// TODO: Add options for parsing live packets or offline pcap file.
class SndPcapParseTask: public TaskBase {
public:
        SndPcapParseTask(): name_("SndPcapParseTask") {}
        ~SndPcapParseTask() override {}

        int Open(TaskContext* ctx) override
        {
                name_ += "(" + ctx->Name() + ")";
                SPDLOG_DEBUG("{}: open task ...", name_);

                return 0;
        }

        int Process(TaskContext* ctx) override
        {
                std::vector<std::unordered_map<std::string, uint32_t>> packets;

                Datum pcap_d = ctx->Inputs().Tag("PATH").Value();
                SPDLOG_DEBUG("{}: Consume Datum: {}", name_, pcap_d);
                auto pcap_opt = pcap_d.Consume<std::string>();
                if (!pcap_opt)
                {
                        SPDLOG_CRITICAL("{}: invalid pcap path", name_);
                        return -1;
                }
                auto pcap = *(pcap_opt.value());
                SPDLOG_DEBUG("{}: pcap: {}", name_, pcap);

                // parse

                SPDLOG_DEBUG("loading pcap file {} ...", pcap);
                char errbuf[PCAP_ERRBUF_SIZE];

                pcap_t* handle = pcap_open_offline(pcap.c_str(), errbuf);
                if (handle == NULL)
                {
                        SPDLOG_ERROR(
                            "fail to execute pcap_open_offline({}), error is {}",
                            pcap,
                            errbuf);
                        return -1;
                }
                pcap_activate(handle);
                pcap_loop(handle, -1, parse_snd_features, (u_char*) &packets);

                SPDLOG_DEBUG("{}: packets size: {}", name_, packets.size());

                ctx->Outputs()[0].AddDatum(Datum(std::move(packets)));

                return 0;
        }

        int Close(TaskContext* ctx) override
        {
                SPDLOG_DEBUG("{}: closing ...", name_);

                return 0;
        }

private:
        std::string name_;
};

REGISTER(SndPcapParseTask);

}   // namespace dni
