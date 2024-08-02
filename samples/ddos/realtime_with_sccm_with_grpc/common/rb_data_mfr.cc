#include "rb_data_mfr.h"

#include <iostream>
#include <netinet/ether.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <sstream>

#include "spdlog/spdlog.h"

#define VLAN_HLEN 4   // for parse pcap with vlan id

namespace dni {

inline int parse_ethtype_vlan(
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
                SPDLOG_INFO("[{}a], proto:0x{:X}", i, ntohs(*h_proto));
                if (ntohs(*h_proto) != ETH_P_8021Q && ntohs(*h_proto) != ETH_P_8021AD)
                {
                        SPDLOG_INFO("neither 802.1q nor 802.1ad");
                        break;
                }
                *l3_off += VLAN_HLEN;
                *h_proto = *(__be16*) ((u_char*) eth_header + (*l3_off) - ETHER_TYPE_LEN);
                SPDLOG_INFO("[{}b], encapsulated proto:0x{:X}", i, ntohs(*h_proto));
        }

        return 0;
}

inline std::string num2hexstring(std::stringstream& sstream, int hex_string_len)
{
        std::string padding = "";
        auto tmp = sstream.str();
        for (int i = 0; i < hex_string_len - (int) tmp.size(); i++)
        {
                padding += "0";
        }

        // padding.clear();
        sstream.clear();
        sstream.str("");

        // SPDLOG_INFO("tmp: {}, padding + tmp: {}", tmp, padding + tmp);

        return padding + tmp;
}

void parse_snd_features(
    std::vector<std::string>& analysis_mfr_data, const u_char* body, int pkt_len)
{
        SPDLOG_INFO("pkt_len: {}, from sccm", pkt_len);
        for (size_t i = 0; i < pkt_len; i++)
        {
                printf("%x  ", body[i]);
        }
        printf("\n");

        // SPDLOG_INFO("struct ether_header: {}", sizeof(struct ether_header));
        // SPDLOG_INFO("struct ip: {}", sizeof(struct ip));

        std::string mfr_header = "";
        std::string mfr_payload = "";
        std::stringstream sstream;
        unsigned long data_len = 0;
        u_char* data = 0;

        struct ether_header* eth_header = (struct ether_header*) body;

        uint16_t h_proto = 0;
        uint64_t l3_off = 0;
        if (parse_ethtype_vlan(eth_header, &h_proto, &l3_off) != 0)
        {
                SPDLOG_INFO("parse_ethtype_vlan failed");

                mfr_header.append(160, '0');

                data = (u_char*) body;
                data_len = pkt_len;
                if (data_len >= (480 / 2))
                {
                        for (size_t i = 0; i < (480 / 2); i++)
                        {
                                sstream << std::hex << (unsigned int) data[i];
                                mfr_payload += num2hexstring(sstream, 2);
                        }
                }
                else
                {
                        for (size_t i = 0; i < data_len; i++)
                        {
                                sstream << std::hex << (unsigned int) data[i];
                                mfr_payload += num2hexstring(sstream, 2);
                        }

                        mfr_payload.append(480 - (int) data_len * 2, '0');
                }

                analysis_mfr_data.emplace_back(std::move(mfr_header + mfr_payload));

                return;
        }

        if (ntohs(h_proto) != ETHERTYPE_IP)
        {
                SPDLOG_INFO("h_proto is not ETHERTYPE_IP");

                mfr_header.append(160, '0');

                data = (u_char*) body;
                data_len = pkt_len;
                if (data_len >= (480 / 2))
                {
                        for (size_t i = 0; i < (480 / 2); i++)
                        {
                                sstream << std::hex << (unsigned int) data[i];
                                mfr_payload += num2hexstring(sstream, 2);
                        }
                }
                else
                {
                        for (size_t i = 0; i < data_len; i++)
                        {
                                sstream << std::hex << (unsigned int) data[i];
                                mfr_payload += num2hexstring(sstream, 2);
                        }

                        mfr_payload.append(480 - (int) data_len * 2, '0');
                }

                analysis_mfr_data.emplace_back(std::move(mfr_header + mfr_payload));

                return;
        }

        struct ip* ip_header = (struct ip*) (body + l3_off);

        // ip version
        sstream << std::hex << ip_header->ip_v;
        mfr_header += num2hexstring(sstream, 1);

        // ihl
        sstream << std::hex << ip_header->ip_hl;
        mfr_header += num2hexstring(sstream, 1);

        // tos
        sstream << std::hex << (unsigned int) (ip_header->ip_tos);
        mfr_header += num2hexstring(sstream, 2);

        // len
        sstream << std::hex << ntohs(ip_header->ip_len);
        mfr_header += num2hexstring(sstream, 4);

        // id
        sstream << std::hex << ntohs(ip_header->ip_id);
        mfr_header += num2hexstring(sstream, 4);

        // off
        sstream << std::hex << ntohs(ip_header->ip_off);
        mfr_header += num2hexstring(sstream, 4);

        // ttl
        sstream << std::hex << (unsigned int) (ip_header->ip_ttl);
        mfr_header += num2hexstring(sstream, 2);

        // proto
        sstream << std::hex << (unsigned int) (ip_header->ip_p);
        mfr_header += num2hexstring(sstream, 2);

        // ip checksum
        sstream << std::hex << ntohs(ip_header->ip_sum);
        mfr_header += num2hexstring(sstream, 4);

        // src ip
        uint32_t src_ip = ntohl(ip_header->ip_src.s_addr);
        sstream << std::hex << (unsigned int) ((src_ip >> 24) & 0xFF);
        mfr_header += num2hexstring(sstream, 2);

        sstream << std::hex << (unsigned int) ((src_ip >> 16) & 0xFF);
        mfr_header += num2hexstring(sstream, 2);

        sstream << std::hex << (unsigned int) ((src_ip >> 8) & 0xFF);
        mfr_header += num2hexstring(sstream, 2);

        sstream << std::hex << (unsigned int) (src_ip & 0xFF);
        mfr_header += num2hexstring(sstream, 2);

        // dst ip
        uint32_t dst_ip = ntohl(ip_header->ip_dst.s_addr);
        sstream << std::hex << (unsigned int) ((dst_ip >> 24) & 0xFF);
        mfr_header += num2hexstring(sstream, 2);

        sstream << std::hex << (unsigned int) ((dst_ip >> 16) & 0xFF);
        mfr_header += num2hexstring(sstream, 2);

        sstream << std::hex << (unsigned int) ((dst_ip >> 8) & 0xFF);
        mfr_header += num2hexstring(sstream, 2);

        sstream << std::hex << (unsigned int) (dst_ip & 0xFF);
        mfr_header += num2hexstring(sstream, 2);

        // ip options
        int ip_option_len = ip_header->ip_hl * 4 - sizeof(struct ip);
        u_char* ip_opt_data = (u_char*) ip_header + sizeof(struct ip);
        SPDLOG_INFO("ip option len: {}", ip_option_len);
        for (size_t i = 0; i < ip_option_len; i++)
        {
                sstream << std::hex << (unsigned int) ip_opt_data[i];
                mfr_header += num2hexstring(sstream, 2);
        }

        u_char proto = ip_header->ip_p;
        SPDLOG_INFO("protocol is : {}", proto);
        switch (proto)
        {
        case IPPROTO_TCP: {
                struct tcphdr* tcp =
                    (struct tcphdr*) ((u_char*) ip_header + ip_header->ip_hl * 4);

                SPDLOG_INFO("src_port = {}", ntohs(tcp->source));
                SPDLOG_INFO("dst_port = {}", ntohs(tcp->dest));

                sstream << std::hex << ntohs(tcp->source);
                mfr_header += num2hexstring(sstream, 4);

                sstream << std::hex << ntohs(tcp->dest);
                mfr_header += num2hexstring(sstream, 4);

                sstream << std::hex << ntohl(tcp->seq);
                mfr_header += num2hexstring(sstream, 8);

                sstream << std::hex << ntohl(tcp->ack_seq);
                mfr_header += num2hexstring(sstream, 8);

                // header length, reserve, flags
                sstream << std::hex << (unsigned int) (tcp->doff & 0xF);
                mfr_header += num2hexstring(sstream, 1);

                sstream << std::hex << (unsigned int) (tcp->res1 & 0xF);
                mfr_header += num2hexstring(sstream, 1);
                sstream << std::hex
                        << (unsigned int) (((tcp->res2 & 0x3) << 2) |
                                           ((tcp->urg & 0x1) << 1) | (tcp->ack & 0x1));
                mfr_header += num2hexstring(sstream, 1);

                sstream << std::hex
                        << (unsigned int) (((tcp->psh & 0x1) << 3) |
                                           ((tcp->rst & 0x1) << 2) |
                                           ((tcp->syn & 0x1) << 1) | (tcp->fin & 0x1));
                mfr_header += num2hexstring(sstream, 1);

                // -----
                sstream << std::hex << ntohs(tcp->window);
                mfr_header += num2hexstring(sstream, 4);

                sstream << std::hex << ntohs(tcp->check);
                mfr_header += num2hexstring(sstream, 4);

                sstream << std::hex << ntohs(tcp->urg_ptr);
                mfr_header += num2hexstring(sstream, 4);

                // options, tcp->doff * 4 - 20 = (n - 5) * 4 bytes
                // so convert every 4 bytes
                // (n - 5) times in for-loop
                // SPDLOG_INFO("struct tcphdr: {}", sizeof(struct tcphdr));
                u_char* opt_data = (u_char*) tcp + sizeof(struct tcphdr);

                // tcp header length, bytes
                uint16_t opt_len = tcp->doff * 4 - sizeof(struct tcphdr);
                SPDLOG_INFO("tcp option len: {}", opt_len);
                for (int i = 0; i < (int) opt_len; i++)
                {
                        sstream << std::hex << (unsigned int) opt_data[i];
                        mfr_header += num2hexstring(sstream, 2);
                }

                // data
                data_len = pkt_len - sizeof(struct ether_header) - ip_header->ip_hl * 4 -
                           tcp->doff * 4;

                SPDLOG_INFO("tcp data len: {}", data_len);
                data = (u_char*) tcp + tcp->doff * 4;

                break;
        }

        case IPPROTO_UDP: {
                struct udphdr* udp =
                    (struct udphdr*) ((u_char*) ip_header + ip_header->ip_hl * 4);

                sstream << std::hex << ntohs(udp->source);
                mfr_header += num2hexstring(sstream, 4);

                sstream << std::hex << ntohs(udp->dest);
                mfr_header += num2hexstring(sstream, 4);

                sstream << std::hex << ntohs(udp->len);
                mfr_header += num2hexstring(sstream, 4);

                sstream << std::hex << ntohs(udp->check);
                mfr_header += num2hexstring(sstream, 4);

                // data
                // SPDLOG_INFO("struct udphdr: {}", sizeof(struct udphdr));
                data_len = pkt_len - sizeof(struct ether_header) - ip_header->ip_hl * 4 -
                           sizeof(struct udphdr);

                SPDLOG_INFO("udp data len: {}", data_len);
                data = (u_char*) udp + sizeof(struct udphdr);

                break;
        }

        case IPPROTO_ICMP: {
                struct icmphdr* icmp =
                    (struct icmphdr*) ((u_char*) ip_header + ip_header->ip_hl * 4);

                sstream << std::hex << (unsigned int) (icmp->type);
                mfr_header += num2hexstring(sstream, 2);

                sstream << std::hex << (unsigned int) (icmp->code);
                mfr_header += num2hexstring(sstream, 2);

                sstream << std::hex << ntohs(icmp->checksum);
                mfr_header += num2hexstring(sstream, 4);

                sstream << std::hex << ntohs(icmp->un.echo.id);
                mfr_header += num2hexstring(sstream, 4);

                sstream << std::hex << ntohs(icmp->un.echo.sequence);
                mfr_header += num2hexstring(sstream, 4);

                // SPDLOG_INFO("struct icmphdr: {}", sizeof(struct icmphdr));
                data_len = pkt_len - sizeof(struct ether_header) - ip_header->ip_hl * 4 -
                           sizeof(struct icmphdr);

                SPDLOG_INFO("icmp data len: {}", data_len);
                data = (u_char*) icmp + sizeof(struct icmphdr);

                break;
        }
        default: {
                // after ip segment and ip option segment, is payload part.
                data_len = pkt_len - sizeof(struct ether_header) - ip_header->ip_hl * 4;

                SPDLOG_INFO("default protocol data len: {}", data_len);

                data = ip_opt_data + ip_option_len;

                break;
        }
        }

        // SPDLOG_INFO(
        //     "orig mfr_header size: {}, mfr_header: {}", mfr_header.size(),
        //     mfr_header);
        if (mfr_header.size() > 160)
        {
                mfr_header = mfr_header.substr(0, 160);
        }
        else
        {
                mfr_header.append(160 - (int) mfr_header.size(), '0');
        }
        // SPDLOG_INFO("160 mfr_header size: {}, {}", mfr_header.size(),
        // mfr_header);

        // convert data to hex string
        if (data_len >= (480 / 2))
        {
                for (size_t i = 0; i < (480 / 2); i++)
                {
                        sstream << std::hex << (unsigned int) data[i];
                        mfr_payload += num2hexstring(sstream, 2);
                }
        }
        else
        {
                for (size_t i = 0; i < data_len; i++)
                {
                        sstream << std::hex << (unsigned int) data[i];
                        mfr_payload += num2hexstring(sstream, 2);
                }

                mfr_payload.append(480 - (int) data_len * 2, '0');
        }

        // SPDLOG_INFO("480 mfr_payload size: {}, {}", mfr_payload.size(),
        // mfr_payload);

        analysis_mfr_data.emplace_back(std::move(mfr_header + mfr_payload));

        return;
}

void get_mfr_bytes(
    std::vector<std::string>& mfr_bytes_list, unsigned char* pktsdata, uint32_t pkts_cnt,
    int batch_size)
{
        SPDLOG_INFO("pkts_cnt: {}, from sccm", pkts_cnt);

        std::vector<std::string> analysis_mfr_data;
        analysis_mfr_data.reserve(batch_size * 5);
        int need_parse_cnt = 0;
        if (pkts_cnt >= batch_size * 5)
        {
                need_parse_cnt = batch_size * 5;
        }
        else
        {
                need_parse_cnt = pkts_cnt;
        }

        unsigned char* data = pktsdata;
        uint16_t pkt_len = *((uint16_t*) data);
        uint32_t i = 0;
        while (i < need_parse_cnt)
        {
                data += 2;
                parse_snd_features(analysis_mfr_data, data, pkt_len);

                data += pkt_len;
                pkt_len = *((uint16_t*) data);

                i++;
        }

        SPDLOG_INFO("analysis_mfr_data size: {}", analysis_mfr_data.size());

        int real_batch_size = analysis_mfr_data.size() / 5;
        SPDLOG_INFO("real_batch_size size: {}", real_batch_size);
        std::string final_data = "";
        for (size_t i = 0; i < real_batch_size; i++)
        {
                final_data.clear();
                for (size_t j = 0; j < 5; j++)
                {
                        final_data += analysis_mfr_data[i * 5 + j];
                }
                mfr_bytes_list.emplace_back(final_data);
        }

        if (real_batch_size < batch_size)
        {
                for (int i = 0; i < batch_size - real_batch_size; i++)
                {
                        final_data.clear();
                        final_data.append((160 + 480) * 5, '0');

                        mfr_bytes_list.emplace_back(final_data);
                }
        }

        SPDLOG_INFO("mfr_bytes_list size: {}", mfr_bytes_list.size());

        return;
}

void get_model_input(std::vector<std::string>& mfr_bytes_list, float* model_np_list)
{
        int dim0_index = 0;
        for (auto&& mfr_item : mfr_bytes_list)
        {
                int pos = 0;
                for (size_t i = 0; i < 40; i++)
                {
                        for (size_t j = 0; j < 40; j++)
                        {
                                *(model_np_list + dim0_index * 1600 + 40 * i + j) =
                                    (std::stoul(mfr_item.substr(pos, 2), 0, 16) / 255.0 -
                                     0.5) /
                                    0.5;

                                pos += 2;
                        }
                }

                dim0_index++;
        }

        // debug
        for (size_t k = 0; k < dim0_index; k++)
        {
                for (size_t i = 0; i < 40; i++)
                {
                        for (size_t j = 0; j < 40; j++)
                        {
                                std::cout << *(model_np_list + k * 1600 + 40 * i + j)
                                          << " ";
                        }

                        std::cout << std::endl;
                }
                std::cout << std::endl;
                std::cout << std::endl;
        }
        std::cout << std::endl;
        std::cout << std::endl;
}

}   // namespace dni