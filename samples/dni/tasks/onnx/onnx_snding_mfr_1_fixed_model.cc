#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

#include "dni/framework/framework.h"
#include "dni/tasks/onnx/onnx.h"
// #include "onnx_defines.h"
#include <iostream>
#include <netinet/ether.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "pcap.h"
#include "spdlog/spdlog.h"

#define VLAN_HLEN 4   // for parse pcap with vlan id

inline int parse_ethtype_vlan(
    struct ether_header* eth_header, uint16_t* h_proto, uint64_t* l3_off)
{
        int i;
        struct vlan_hdr* vhdr = NULL;

        *l3_off = ETH_HLEN;

        *h_proto = eth_header->ether_type;
        if (ntohs(*h_proto) < ETH_P_802_3_MIN)
        {
                spdlog::info(
                    "proto 0x{:X} is not eth-ii packet, not supported", ntohs(*h_proto));
                return -1;   // only Ethernet-II supported
        }

        // #pragma unroll
        for (i = 0; i < 2; i++)
        {
                spdlog::trace("[{}a], proto:0x{:X}", i, ntohs(*h_proto));
                if (ntohs(*h_proto) != ETH_P_8021Q && ntohs(*h_proto) != ETH_P_8021AD)
                {
                        spdlog::trace("neither 802.1q nor 802.1ad");
                        break;
                }
                *l3_off += VLAN_HLEN;
                *h_proto = *(__be16*) ((u_char*) eth_header + (*l3_off) - ETHER_TYPE_LEN);
                spdlog::trace("[{}b], encapsulated proto:0x{:X}", i, ntohs(*h_proto));
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

        // spdlog::info("tmp: {}, padding + tmp: {}", tmp, padding + tmp);

        return padding + tmp;
}

void parse_snd_features(u_char* args, const struct pcap_pkthdr* hdr, const u_char* body)
{
        // spdlog::info("caplen: {}, len: {}", ntohl(hdr->caplen), ntohl(hdr->len));
        spdlog::trace("caplen: {}, len: {}", hdr->caplen, hdr->len);
        // spdlog::info("struct ether_header: {}", sizeof(struct ether_header));
        // spdlog::info("struct ip: {}", sizeof(struct ip));

        std::vector<std::string>* analysis_mfr_data = (std::vector<std::string>*) args;

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
                spdlog::info("parse_ethtype_vlan failed");

                mfr_header.append(160, '0');

                data = (u_char*) body;
                data_len = hdr->caplen;
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

                analysis_mfr_data->emplace_back(std::move(mfr_header + mfr_payload));

                return;
        }

        if (ntohs(h_proto) != ETHERTYPE_IP)
        {
                spdlog::info("h_proto is not ETHERTYPE_IP");

                mfr_header.append(160, '0');

                data = (u_char*) body;
                data_len = hdr->caplen;
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

                analysis_mfr_data->emplace_back(std::move(mfr_header + mfr_payload));

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
        spdlog::debug("ip option len: {}", ip_option_len);
        for (size_t i = 0; i < ip_option_len; i++)
        {
                sstream << std::hex << (unsigned int) ip_opt_data[i];
                mfr_header += num2hexstring(sstream, 2);
        }

        u_char proto = ip_header->ip_p;
        spdlog::trace("protocol is : {}", proto);
        switch (proto)
        {
        case IPPROTO_TCP: {
                struct tcphdr* tcp =
                    (struct tcphdr*) ((u_char*) ip_header + ip_header->ip_hl * 4);

                spdlog::trace("src_port = {}", ntohs(tcp->source));
                spdlog::trace("dst_port = {}", ntohs(tcp->dest));

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
                // spdlog::info("struct tcphdr: {}", sizeof(struct tcphdr));
                u_char* opt_data = (u_char*) tcp + sizeof(struct tcphdr);

                // tcp header length, bytes
                uint16_t opt_len = tcp->doff * 4 - sizeof(struct tcphdr);
                spdlog::debug("tcp option len: {}", opt_len);
                for (int i = 0; i < (int) opt_len; i++)
                {
                        sstream << std::hex << (unsigned int) opt_data[i];
                        mfr_header += num2hexstring(sstream, 2);
                }

                // data
                data_len = hdr->caplen - sizeof(struct ether_header) -
                           ip_header->ip_hl * 4 - tcp->doff * 4;

                spdlog::trace("tcp data len: {}", data_len);
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
                // spdlog::info("struct udphdr: {}", sizeof(struct udphdr));
                data_len = hdr->caplen - sizeof(struct ether_header) -
                           ip_header->ip_hl * 4 - sizeof(struct udphdr);

                spdlog::trace("udp data len: {}", data_len);
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

                // spdlog::info("struct icmphdr: {}", sizeof(struct icmphdr));
                data_len = hdr->caplen - sizeof(struct ether_header) -
                           ip_header->ip_hl * 4 - sizeof(struct icmphdr);

                spdlog::trace("icmp data len: {}", data_len);
                data = (u_char*) icmp + sizeof(struct icmphdr);

                break;
        }
        default: {
                // after ip segment and ip option segment, is payload part.
                data_len =
                    hdr->caplen - sizeof(struct ether_header) - ip_header->ip_hl * 4;

                spdlog::debug("default protocol data len: {}", data_len);

                data = ip_opt_data + ip_option_len;

                break;
        }
        }

        // spdlog::info(
        //     "orig mfr_header size: {}, mfr_header: {}", mfr_header.size(), mfr_header);
        if (mfr_header.size() > 160)
        {
                mfr_header = mfr_header.substr(0, 160);
        }
        else
        {
                mfr_header.append(160 - (int) mfr_header.size(), '0');
        }
        // spdlog::info("160 mfr_header size: {}, {}", mfr_header.size(), mfr_header);

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

        // spdlog::info("480 mfr_payload size: {}, {}", mfr_payload.size(), mfr_payload);

        analysis_mfr_data->emplace_back(std::move(mfr_header + mfr_payload));

        return;
}

void get_mfr_bytes(
    std::vector<std::string>& mfr_bytes_list,
    const std::string& pcap,
    int batch_size = 64)
{
        spdlog::debug("loading pcap file {} ...", pcap);
        char errbuf[PCAP_ERRBUF_SIZE];

        pcap_t* handle = pcap_open_offline(pcap.c_str(), errbuf);
        if (handle == NULL)
        {
                spdlog::error(
                    "fail to execute pcap_open_offline({}), error is {}", pcap, errbuf);
                return;
        }

        // analysis_mfr_data.resize(batch_size * 5);
        std::vector<std::string> analysis_mfr_data;
        pcap_activate(handle);
        pcap_loop(
            handle, batch_size * 5, parse_snd_features, (u_char*) &analysis_mfr_data);
        pcap_close(handle);

        int analysis_mfr_data_size = analysis_mfr_data.size();
        spdlog::debug("analysis_mfr_data size: {}", analysis_mfr_data_size);

        // int cnt = 0;
        // for (auto&& mfr : analysis_mfr_data)
        // {
        //         cnt++;
        //         // spdlog::info("{}, {}", cnt, mfr);
        // }

        int real_batch_size = analysis_mfr_data_size / 5;
        spdlog::debug("real_batch_size size: {}", real_batch_size);
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

        return;
}

void get_model_input(std::vector<std::string>& mfr_bytes_list, float* model_np_list)
{
        int dim0_index = 0;
        for (auto&& mfr_item : mfr_bytes_list)
        {
                int pos = 0;
                // mfr_input_tensor input_tensor;
                for (size_t i = 0; i < 40; i++)
                {
                        for (size_t j = 0; j < 40; j++)
                        {
                                // model_np_list[dim0_index][0][i][j] =
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

void userPrepareInputData1(
    std::vector<std::vector<float>*>& input_data, float* model_np_list, int64_t total)
{
        std::vector<float>* input_tensor = new std::vector<float>();
        for (int64_t i = 0; i < total; i++)
        {
                input_tensor->push_back(model_np_list[i]);
                // input_tensor->push_back(0.7);
        }

        input_data.push_back(input_tensor);
}

void inject_after1(dni::Graph* g, float* model_np_list, int64_t total)
{
        std::vector<std::vector<float>*> input_data;
        userPrepareInputData1(input_data, model_np_list, total);   // user call

        g->AddDatumToInputStream("data", dni::Datum(input_data));
}

std::vector<std::string> label_relation_dict = {
    "BROWSING", "CHAT", "DDoS-hping3", "DDoS-pktgen", "DDoS-trafgen",
    "FTP",      "MAIL", "Streaming",   "VOIP",        "p2p"};

std::string get_label_from_infer_ret(
    const std::vector<std::vector<std::vector<float_t>>>& infer_ret)
{
        // only one output in this onnx model
        auto output_data = infer_ret[0];
        std::vector<int> model_output;
        for (auto&& od : output_data)
        {
                int maxPos = std::max_element(od.begin(), od.end()) - od.begin();
                model_output.push_back(maxPos);
        }
        spdlog::info("model_output: {}", model_output);

        std::vector<int> model_output_count;
        model_output_count.resize(label_relation_dict.size());
        for (auto&& maxPos : model_output)
        {
                model_output_count[maxPos]++;
        }
        spdlog::info("model_output_count: {}", model_output_count);

        int predicted_class_id =
            std::max_element(model_output_count.begin(), model_output_count.end()) -
            model_output_count.begin();

        spdlog::info("predicted_class_id: {}", predicted_class_id);

        auto predicted_class = label_relation_dict[predicted_class_id];

        return predicted_class;
}

int main()
{
        // spdlog::set_level(spdlog::level::trace);
        spdlog::set_level(spdlog::level::info);

        const std::string& proto = R"pb(
                type: "Onnx-MFR-Model"

                input_stream: "GIn_Data:0:data"
                output_stream: "GOut_Ret:0:ret"

                node {
                  name: "A"
                  task: "OnnxSNDingMFRTask"
                  input_stream: "GIn_Data:0:data"
                  output_stream: "GOut_Ret:0:ret"

                  options {
                    [type.asnapis.io/dni.OnnxTaskOptions] {
                      model_path: "samples/dni/tasks/onnx/testdata/snding_finetuned_model_200_20240724163131.onnx"
                    }
                  }
                }
        )pb";

        std::vector<std::string> mfr_bytes_list;
        // std::string pcap =
        //     "/home/zhouxu/works/2024/50_dni/pcap/mfr/tcp-35packets.pcap";

        // std::string pcap = "/home/zhouxu/works/2024/50_dni/pcap/mfr/orig/udp.pcap";
        // std::string pcap = "/home/zhouxu/works/2024/50_dni/pcap/mfr/orig/tcp.pcap";
        // std::string pcap = "/home/zhouxu/works/2024/50_dni/pcap/mfr/orig/icmp.pcap";

        std::string pcap =
            "/home/zhouxu/works/2024/50_dni/0802/log/0806-hping3-udp---2.pcap";

        // std::string pcap =
        //     "/home/zhouxu/works/2024/50_dni/pcap/mfr/test001_no_ip_header.pcap";

        int batch_size = 64;
        get_mfr_bytes(mfr_bytes_list, pcap, batch_size);

        // int cnt = 0;
        // for (auto&& mfr : mfr_bytes_list)
        // {
        //         cnt++;
        //         spdlog::info("{}, {}", cnt, mfr);
        // }
        // spdlog::info("\n");

        float* model_np_list = (float*) malloc(batch_size * 1 * 40 * 40 * sizeof(float));
        memset(model_np_list, 0, batch_size * 1 * 40 * 40 * sizeof(float));
        get_model_input(mfr_bytes_list, model_np_list);

        auto gc = dni::ParseStringToGraphConfig(proto);
        if (!gc)
        {
                spdlog::error("invalid pbtxt config: {}", proto);
                return -1;
        }
        spdlog::debug(
            "GraphConfig: {}, node size: {}", gc.value().type(), gc.value().node_size());

        dni::Graph* g = new dni::Graph(gc.value());

        std::string out = "ret";

        spdlog::debug("Create ObserveOutputStream: {}", out);
        g->ObserveOutputStream(out);

        g->PrepareForRun();
        inject_after1(g, model_np_list, (int64_t) batch_size * 1 * 40 * 40);

        g->RunOnce();

        g->Wait();

        auto ret = g->GetResult<std::vector<std::vector<std::vector<float_t>>>>(out);

        // spdlog::info("Gout {} result is: {}", out, ret);
        auto predicted_class = get_label_from_infer_ret(ret);
        spdlog::info("Predicted data: {},  Predicted class: {}", pcap, predicted_class);

        g->Finish();

        spdlog::info("main over");

        return 0;
}
