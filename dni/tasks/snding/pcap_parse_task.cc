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
                    "proto 0x{:X} is not eth-ii packet, not supported", ntohs(*h_proto));
                return -1;   // only Ethernet-II supported
        }

        // #pragma unroll
        for (i = 0; i < 2; i++)
        {
                SPDLOG_TRACE("[{}a], proto:0x{:X}", i, ntohs(*h_proto));
                if (ntohs(*h_proto) != ETH_P_8021Q && ntohs(*h_proto) != ETH_P_8021AD)
                {
                        SPDLOG_TRACE("neither 802.1q nor 802.1ad");
                        break;
                }
                *l3_off += VLAN_HLEN;
                *h_proto = *(__be16*) ((u_char*) eth_header + (*l3_off) - ETHER_TYPE_LEN);
                SPDLOG_TRACE("[{}b], encapsulated proto:0x{:X}", i, ntohs(*h_proto));
        }

        return 0;
}

void parse_snd_features(u_char* args, const struct pcap_pkthdr* hdr, const u_char* body)
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
                            (struct tcphdr*) ((u_char*) ip_header + ip_header->ip_hl * 4);
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
                            (struct udphdr*) ((u_char*) ip_header + ip_header->ip_hl * 4);
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

                SPDLOG_TRACE(
                    "Packet total len: {}, capture len: {}", hdr->len, hdr->caplen);

                // packet["Length"] = hdr->len;
                packet["Length"] = ntohs(ip_header->ip_len);

                packet["Protocol"] = proto;

                auto packets =
                    (std::vector<std::unordered_map<std::string, uint32_t>>*) args;
                packets->push_back(packet);
        }

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
