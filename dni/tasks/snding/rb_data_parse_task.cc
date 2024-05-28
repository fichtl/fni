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

void parse_snd_features(
    std::vector<std::unordered_map<std::string, uint32_t>>& packets, const u_char* body)
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

void parse_header(
    unsigned char* rbdata, std::string& nic_name, uint64_t& ts,
    std::vector<uint32_t>& pkts_stats, int& offset)
{
        offset = 0;

        unsigned char* slot = rbdata;
        uint16_t nic_name_len = *((uint16_t*) slot);
        slot += 2;
        nic_name = std::string((char*) slot, nic_name_len);

        slot += nic_name_len;

        ts = *((uint64_t*) slot);
        slot += 8;

        for (size_t i = 0; i < 9; i++)
        {
                pkts_stats.push_back(*((uint32_t*) slot));
                slot += 4;
        }

        offset = slot - rbdata;
}

// TODO: Add options for parsing live packets or offline pcap file.
class RBDataParseTask: public TaskBase {
public:
        RBDataParseTask(): name_("RBDataParseTask") {}
        ~RBDataParseTask() override {}

        int Open(TaskContext* ctx) override
        {
                name_ += "(" + ctx->Name() + ")";
                SPDLOG_DEBUG("{}: open task ...", name_);

                return 0;
        }

        int Process(TaskContext* ctx) override
        {
                // TODO: if need payload of packet
                std::vector<std::unordered_map<std::string, uint32_t>> packets;
                std::string nic_name;
                uint64_t ts;
                std::vector<uint32_t> pkts_stats;
                int offset;   // for jump to packets data

                Datum rbdata_d = ctx->Inputs().Tag("RBData").Value();
                SPDLOG_DEBUG("{}: Consume Datum: {}", name_, rbdata_d);
                auto rbdata_opt = rbdata_d.Consume<unsigned char*>();
                if (!rbdata_opt)
                {
                        SPDLOG_CRITICAL("{}: invalid pcap path", name_);
                        return -1;
                }
                auto rbdata = *(rbdata_opt.value());
                // SPDLOG_DEBUG("{}: rbdata: 0x{:X}", name_, (void*) rbdata);

                // parse
                parse_header(rbdata, nic_name, ts, pkts_stats, offset);

                // if countTotal is 0, stop
                if (pkts_stats[0] == 0)
                {
                        SPDLOG_INFO("No packets");
                        return -100;
                }

                SPDLOG_DEBUG("countTotal: {}", pkts_stats[0]);

                // if countTotal > 10000, sccm will apply 10000 packets
                // else, will apply countTotal packets
                uint32_t pkts_cnt = (pkts_stats[0] > 10000 ? 10000 : pkts_stats[0]);
                parse_packets(rbdata + offset, pkts_cnt, packets);

                SPDLOG_DEBUG("{}: parsed_packets size: {}", name_, packets.size());
                SPDLOG_DEBUG("{}: host_nic_name: {}", name_, nic_name);
                SPDLOG_DEBUG("{}: ts: {}", name_, ts);
                SPDLOG_DEBUG("{}: statistics: {}", name_, pkts_stats);

                ctx->Outputs()
                    .Get("ParsedPackets", 0)
                    .AddDatum(Datum(std::move(packets)));
                ctx->Outputs().Get("NIC", 0).AddDatum(Datum(std::move(nic_name)));
                ctx->Outputs().Get("Timestamp", 0).AddDatum(Datum(std::move(ts)));
                ctx->Outputs().Get("Statistic", 0).AddDatum(Datum(std::move(pkts_stats)));

                SPDLOG_DEBUG("{}: calc over .......", name_);

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

REGISTER(RBDataParseTask);

}   // namespace dni
