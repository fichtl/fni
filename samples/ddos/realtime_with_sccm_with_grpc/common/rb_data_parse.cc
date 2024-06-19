#include "rb_data_parse.h"

#ifdef __APPLE__
#        include <netinet/if_ether.h>
#else
#        include <netinet/ether.h>
#endif
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

#include "pcap.h"
#include "spdlog/spdlog.h"

#ifdef __APPLE__

// map macros/definitions from MacOS to Linux
#        define ETH_HLEN        ((ETHER_HDR_LEN))
#        define ETH_P_8021Q     ((ETHERTYPE_VLAN))
#        define ETH_P_802_3_MIN 0x0600
#        define ETH_P_8021AD    0x88A8

#        ifndef __bitwise
#                ifdef __CHECKER__
#                        define __bitwise__ __attribute__((bitwise))
#                else
#                        define __bitwise__
#                endif
#                define __bitwise __bitwise__
#        endif

typedef u_int16_t __bitwise __be16;

#endif

#define VLAN_HLEN 4   // for parse pcap with vlan id

namespace dni {

        int parse_ethtype_vlan(
            struct ether_header* eth_header, uint16_t* h_proto, uint64_t* l3_off)
        {
                int i;
                struct vlan_hdr* vhdr = NULL;

                *l3_off = ETH_HLEN;

                *h_proto = eth_header->ether_type;
                if (ntohs(*h_proto) < ETH_P_802_3_MIN)
                {
                        SPDLOG_INFO(
                            "proto 0x{:X} is not eth-ii packet, not supported",
                            ntohs(*h_proto));
                        return -1;   // only Ethernet-II supported
                }

                // #pragma unroll
                for (i = 0; i < 2; i++)
                {
                        SPDLOG_TRACE("[{}a], proto:0x{:X}", i, ntohs(*h_proto));
                        if (ntohs(*h_proto) != ETH_P_8021Q &&
                            ntohs(*h_proto) != ETH_P_8021AD)
                        {
                                SPDLOG_TRACE("neither 802.1q nor 802.1ad");
                                break;
                        }
                        *l3_off += VLAN_HLEN;
                        *h_proto = *(__be16*) ((u_char*) eth_header + (*l3_off) -
                                               ETHER_TYPE_LEN);
                        SPDLOG_TRACE(
                            "[{}b], encapsulated proto:0x{:X}", i, ntohs(*h_proto));
                }

                return 0;
        }

        void parse_snd_features(
            std::vector<std::unordered_map<std::string, uint32_t>>& packets,
            const u_char* body)
        {
                struct ether_header* eth_header = (struct ether_header*) body;

                uint16_t h_proto = 0;
                uint64_t l3_off = 0;
                if (parse_ethtype_vlan(eth_header, &h_proto, &l3_off) != 0)
                {
                        return;
                }

                if (ntohs(h_proto) == ETHERTYPE_IP)
                {
                        std::unordered_map<std::string, uint32_t> packet;

                        struct ip* ip_header = (struct ip*) (body + l3_off);

                        char ipv4[INET_ADDRSTRLEN];
                        // #ifdef __APPLE__
                        inet_ntop(AF_INET, &ip_header->ip_src, ipv4, sizeof(ipv4));
                        SPDLOG_TRACE("src ip addr is: {}", ipv4);
                        packet["SIP"] = ntohl(ip_header->ip_src.s_addr);

                        inet_ntop(AF_INET, &ip_header->ip_dst, ipv4, sizeof(ipv4));
                        SPDLOG_TRACE("dst ip addr is: {}", ipv4);
                        packet["DIP"] = ntohl(ip_header->ip_dst.s_addr);

                        u_char proto = ip_header->ip_p;
                        SPDLOG_TRACE("protocol is : {}", proto);
                        switch (proto)
                        {
                        case IPPROTO_TCP: {
                                struct tcphdr* tcp =
                                    (struct tcphdr*) ((u_char*) ip_header +
                                                      ip_header->ip_hl * 4);
#ifdef __APPLE__
                                SPDLOG_TRACE("src_port = {}", ntohs(tcp->th_sport));
                                packet["SPort"] = ntohs(tcp->th_sport);

                                SPDLOG_TRACE("dst_port = {}", ntohs(tcp->th_dport));
                                packet["DPort"] = ntohs(tcp->th_dport);
#else
                                SPDLOG_TRACE("src_port = {}", ntohs(tcp->source));
                                packet["SPort"] = ntohs(tcp->source);

                                SPDLOG_TRACE("dst_port = {}", ntohs(tcp->dest));
                                packet["DPort"] = ntohs(tcp->dest);
#endif

                                SPDLOG_TRACE("-------------------------------\n\n");

                                break;
                        }

                        case IPPROTO_UDP: {
                                struct udphdr* udp =
                                    (struct udphdr*) ((u_char*) ip_header +
                                                      ip_header->ip_hl * 4);
#ifdef __APPLE__
                                SPDLOG_TRACE("src_port = {}", ntohs(udp->uh_sport));
                                packet["SPort"] = ntohs(udp->uh_sport);

                                SPDLOG_TRACE("dst_port = {}", ntohs(udp->uh_dport));
                                packet["DPort"] = ntohs(udp->uh_dport);
#else
                                SPDLOG_TRACE("src_port = {}", ntohs(udp->source));
                                packet["SPort"] = ntohs(udp->source);

                                SPDLOG_TRACE("dst_port = {}", ntohs(udp->dest));
                                packet["DPort"] = ntohs(udp->dest);
#endif

                                SPDLOG_TRACE("-------------------------------\n\n");

                                break;
                        }

                        case IPPROTO_ICMP: {
                                break;
                        }

                        default: return;
                        }

                        packet["Length"] = ntohs(ip_header->ip_len);

                        packet["Protocol"] = proto;

                        packets.push_back(packet);
                }

                return;
        }

        void parse_packets(
            unsigned char* pktsdata, uint32_t cnt,
            std::vector<std::unordered_map<std::string, uint32_t>>& packets)
        {
                unsigned char* data = pktsdata;
                uint16_t pkt_len = *((uint16_t*) data);
                uint32_t i = 0;

                while (i < cnt)
                {
                        data += 2;
                        parse_snd_features(packets, data);

                        data += pkt_len;
                        pkt_len = *((uint16_t*) data);

                        i++;
                }
        }

        void parse_header(unsigned char* rbdata, RBDataHeader& rb_header, int& offset)
        {
                offset = 0;

                unsigned char* slot = rbdata;
                uint16_t nic_name_len = *((uint16_t*) slot);
                slot += 2;
                rb_header.host_nic_name = std::string((char*) slot, nic_name_len);

                slot += nic_name_len;

                rb_header.ts = *((uint64_t*) slot);
                slot += 8;

                for (size_t i = 0; i < 9; i++)
                {
                        rb_header.pkts_stats.push_back(*((uint32_t*) slot));
                        slot += 4;
                }

                for (size_t i = 0; i < 4; i++)
                {
                        rb_header.netdev_stats.push_back(*((double*) slot));
                        slot += 8;
                }
                rb_header.speed = *((int64_t*) slot);
                slot += 8;

                rb_header.cur_cpu = *((double*) slot);
                slot += 8;
                for (size_t i = 0; i < 5; i++)
                {
                        rb_header.resource_stats.push_back(*((int64_t*) slot));
                        slot += 8;
                }

                rb_header.nic_ip = *((uint32_t*) slot);
                slot += 4;

                rb_header.mgr_ip = *((uint32_t*) slot);
                slot += 4;

                for (size_t i = 0; i < 13; i++)
                {
                        rb_header.additional_stats.push_back(*((int64_t*) slot));
                        slot += 8;
                }

                offset = slot - rbdata;
        }
}   // namespace dni