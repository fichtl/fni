#include "daq.h"

#ifdef __APPLE__
#        include <netinet/if_ether.h>
#else
#        include <netinet/ether.h>
#endif
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <pcap.h>
#include <unordered_map>
#include <vector>

#include "spdlog/spdlog.h"

namespace dni {

void parse_single_packet(u_char* args, const struct pcap_pkthdr* hdr, const u_char* body)
{
        auto packets = (std::vector<std::unordered_map<std::string, uint32_t>>*) args;
        std::unordered_map<std::string, uint32_t> packet;

        SPDLOG_DEBUG("Packet capture length: {}", hdr->caplen);
        SPDLOG_DEBUG("Packet total length {}", hdr->len);

        packet["Length"] = hdr->len;

#ifdef __APPLE__

        struct ether_header* eth_header = (struct ether_header*) body;

        if (ntohs(eth_header->ether_type) == ETHERTYPE_IP)
        {
                struct ip* ip_header = (struct ip*) (body + sizeof(struct ether_header));

                char ipv4[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &ip_header->ip_src, ipv4, sizeof(ipv4));
                SPDLOG_DEBUG("src ip addr is: {}", ipv4);
                packet["SIP"] = ntohl(ip_header->ip_src.s_addr);

                inet_ntop(AF_INET, &ip_header->ip_dst, ipv4, sizeof(ipv4));
                SPDLOG_DEBUG("dst ip addr is: {}", ipv4);
                packet["DIP"] = ntohl(ip_header->ip_dst.s_addr);

                SPDLOG_DEBUG("protocol is : {}", ip_header->ip_p);
                switch (ip_header->ip_p)
                {
                case IPPROTO_TCP: {
                        struct tcphdr* tcp =
                            (struct tcphdr*) ((u_char*) ip_header + ip_header->ip_hl * 4);
                        SPDLOG_DEBUG("src_port = {}", ntohs(tcp->th_sport));
                        packet["SPort"] = ntohs(tcp->th_sport);

                        SPDLOG_DEBUG("dst_port = {}", ntohs(tcp->th_dport));
                        packet["DPort"] = ntohs(tcp->th_dport);

                        SPDLOG_DEBUG("-------------------------------\n\n");
                        packet["Protocol"] = 6;

                        break;
                }

                case IPPROTO_UDP: {
                        struct udphdr* udp =
                            (struct udphdr*) ((u_char*) ip_header + ip_header->ip_hl * 4);
                        SPDLOG_DEBUG("src_port = {}", ntohs(udp->uh_sport));
                        packet["SPort"] = ntohs(udp->uh_sport);

                        SPDLOG_DEBUG("dst_port = {}", ntohs(udp->uh_dport));
                        packet["DPort"] = ntohs(udp->uh_dport);

                        SPDLOG_DEBUG("-------------------------------\n\n");
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
                        break;
                }

                default: return;
                }
        }

#endif
        packets->push_back(packet);

        return;
}

}   // namespace dni
