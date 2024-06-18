#include <chrono>
#include <fcntl.h>
#include <iostream>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

#include "dni/framework/framework.h"
#include "dni/tasks/snding/snding_defines.h"
#include "fmt/format.h"
#include "rb_data_parse.h"
#include "spdlog/spdlog.h"

#define SHM_KEY 0x1234

// ring buffer
typedef struct {
        volatile uint32_t head;
        volatile uint32_t tail;
        size_t size;
} ringbuffer_t;

uint8_t* buffer_create(size_t sz)
{
        int shm_id;

        shm_id = shmget(SHM_KEY, sz, IPC_CREAT | 0666);
        if (shm_id == -1)
        {
                perror("parent: shmget SHM_KEY");
                return NULL;
        }

        void* addr = shmat(shm_id, NULL, 0);
        if (addr == (void*) -1)
        {
                perror("parent: shmat SHM_KEY");
                return NULL;
        }

        return (uint8_t*) addr;
}

void buffer_free(uint8_t* buffer)
{
        if (shmdt((const void*) (buffer)) == -1)
        {
                perror("recv: shmdt buffer");
                return;
        }
}

dni::Graph* start_graph1(const std::string& protoPath1)
{
        auto gc = dni::LoadTextprotoFile(protoPath1);
        if (!gc)
        {
                spdlog::error("invalid pbtxt config: {}", protoPath1);
                return nullptr;
        }

        dni::Graph* g = new dni::Graph(gc.value());

        std::string out = "packet_abnormal";
        spdlog::debug("Create ObserveOutputStream: {}", out);
        g->ObserveOutputStream(out);

        std::string out1 = "netdev_abnormal";
        spdlog::debug("Create ObserveOutputStream: {}", out1);
        g->ObserveOutputStream(out1);

        std::string out2 = "resource_abnormal";
        spdlog::debug("Create ObserveOutputStream: {}", out2);
        g->ObserveOutputStream(out2);

        std::string netdev_inMbps = "netdev_inMbps";
        g->ObserveOutputStream(netdev_inMbps);

        std::string netdev_inKpps = "netdev_inKpps";
        g->ObserveOutputStream(netdev_inKpps);

        std::string out3 = "abnormal_res";
        spdlog::debug("Create ObserveOutputStream: {}", out3);
        g->ObserveOutputStream(out3);

        g->PrepareForRun();

        spdlog::info("start_graph1.");

        return g;
}

dni::Graph* start_graph2(const std::string& protoPath2)
{
        auto gc = dni::LoadTextprotoFile(protoPath2);
        if (!gc)
        {
                spdlog::error("invalid pbtxt config: {}", protoPath2);
                return nullptr;
        }

        dni::Graph* g = new dni::Graph(gc.value());

        std::string attack_result = "attack_res";
        spdlog::debug("Create ObserveOutputStream: {}", attack_result);
        g->ObserveOutputStream(attack_result);

        std::string sip_count = "sip_count";
        spdlog::debug("Create ObserveOutputStream: {}", sip_count);
        g->ObserveOutputStream(sip_count);

        g->PrepareForRun();

        spdlog::info("start_graph2.");

        return g;
}

dni::Graph* start_graph3(const std::string& protoPath3)
{
        auto gc = dni::LoadTextprotoFile(protoPath3);
        if (!gc)
        {
                spdlog::error("invalid pbtxt config: {}", protoPath3);
                return nullptr;
        }

        dni::Graph* g = new dni::Graph(gc.value());

        std::string out = "dms_rules";
        g->ObserveOutputStream(out);

        g->PrepareForRun();

        spdlog::info("start_graph3.");

        return g;
}

struct ingressData {
        // std::string pcapNameStr;
        double incrbytesRecv;
        double incrpacketRecv;
};

struct anomalyResult {
        int abnormal_number;
        // std::string pcapName;
        double incrbytesRecv;
        double incrpacketRecv;
        double inMbps_score;
        double inKpps_score;
};

std::vector<std::string> g1_inputs_packets = {
    "countTotal",           "countType_length",     "countType_srcIP",
    "countType_dstIP",      "countType_protocol",   "countType_srcPortTcp",
    "countType_dstPortTcp", "countType_srcPortUdp", "countType_dstPortUdp"};

std::vector<std::string> g1_inputs_packets_cond = {
    "",
    "cond_countType_length",
    "cond_countType_srcIP",
    "cond_countType_dstIP",
    "cond_countType_protocol",
    "cond_countType_srcPortTcp",
    "cond_countType_dstPortTcp",
    "cond_countType_srcPortUdp",
    "cond_countType_dstPortUdp"};

std::vector<std::string> g1_inputs_netdev = {"inMbps", "inKpps", "outMbps", "outKpps"};

std::vector<std::string> g1_inputs_resource = {
    "incr_mem_swap", "incr_mem_virtual", "incr_tcpconn_full", "incr_tcpconn_semi",
    "incr_tcpconn_total"};

ingressData g1_inject_after(dni::Graph* g, dni::RBDataHeader& rb_header)
{
        // packets
        auto countTotal = rb_header.pkts_stats[0];
        std::vector<double> cond = {double(countTotal)};

        g->AddDatumToInputStream(g1_inputs_packets[0], dni::Datum(double(countTotal)));

        for (size_t i = 1; i < 9; i++)
        {
                g->AddDatumToInputStream(
                    g1_inputs_packets[i], dni::Datum(double(rb_header.pkts_stats[i])));
                g->AddDatumToInputStream(g1_inputs_packets_cond[i], dni::Datum(cond));
        }

        // netdev
        for (size_t i = 0; i < 4; i++)
        {
                g->AddDatumToInputStream(
                    g1_inputs_netdev[i], dni::Datum(rb_header.netdev_stats[i]));
        }

        // resource
        g->AddDatumToInputStream("cur_cpu", dni::Datum(rb_header.cur_cpu));
        for (size_t i = 0; i < 5; i++)
        {
                g->AddDatumToInputStream(
                    g1_inputs_resource[i],
                    dni::Datum(double(rb_header.resource_stats[i])));
        }

        ingressData ingress_data;
        ingress_data.incrbytesRecv = rb_header.netdev_stats[0];
        ingress_data.incrpacketRecv = rb_header.netdev_stats[1];

        return ingress_data;
}

anomalyResult g1_anomaly_detect(dni::Graph* g, dni::RBDataHeader& rb_header)
{
        std::string out = "packet_abnormal";
        std::string out1 = "netdev_abnormal";
        std::string out2 = "resource_abnormal";
        std::string out3 = "abnormal_res";
        std::string netdev_inMbps = "netdev_inMbps";
        std::string netdev_inKpps = "netdev_inKpps";

        ingressData ingress_data = g1_inject_after(g, rb_header);
        g->RunOnce();
        g->Wait();

        auto ret = g->GetResult<double_t>(out);
        spdlog::info("Nout {} result is: {}", out, ret);

        auto ret1 = g->GetResult<double_t>(out1);
        spdlog::info("Nout {} result is: {}", out1, ret1);

        auto ret2 = g->GetResult<double_t>(out2);
        spdlog::info("Nout {} result is: {}", out2, ret2);

        auto ret3 = g->GetResult<int>(out3);
        spdlog::info("G1out {} result is: {}", out3, ret3);

        auto inMbps_score = g->GetResult<double_t>(netdev_inMbps);
        auto inKpps_score = g->GetResult<double_t>(netdev_inKpps);

        anomalyResult anomaly_result;
        anomaly_result.abnormal_number = ret3;
        anomaly_result.incrbytesRecv = ingress_data.incrbytesRecv;
        anomaly_result.inMbps_score = inMbps_score;
        anomaly_result.inKpps_score = inKpps_score;
        anomaly_result.incrpacketRecv = ingress_data.incrpacketRecv;

        g->ClearResult();

        return anomaly_result;
}

void g2_inject_after(
    dni::Graph* g, std::vector<std::unordered_map<std::string, uint32_t>>& packets)
{
        g->AddDatumToInputStream("parsed_packets", dni::Datum(packets));
}

int g2_attack_detect(
    dni::Graph* g, std::vector<std::unordered_map<std::string, uint32_t>>& packets,
    std::unordered_map<uint32_t, int>& sip_count_ret)
{
        std::string attack_result = "attack_res";
        std::string sip_count = "sip_count";

        g2_inject_after(g, packets);
        g->RunOnce();
        g->Wait();

        auto attack_number = g->GetResult<double_t>(attack_result);
        spdlog::info("G2out {} result is: {}", attack_result, attack_number);

        sip_count_ret = g->GetResult<std::unordered_map<uint32_t, int>>(sip_count);
        spdlog::info("G2out {} result size is: {}", sip_count, sip_count_ret.size());

        g->ClearResult();

        return attack_number;
}

void g3_inject_after(
    dni::Graph* g, std::vector<std::unordered_map<std::string, uint32_t>>& packets,
    std::vector<uint32_t> all_known_ips, std::vector<double> netdevs,
    std::string host_nic_name, std::unordered_map<uint32_t, int>& sip_count_ret)
{
        g->AddDatumToInputStream("parsed_packets", dni::Datum(packets));

        g->AddDatumToInputStream("all_known_ips", dni::Datum(all_known_ips));

        g->AddDatumToInputStream("host_nic_name", dni::Datum(std::string(host_nic_name)));

        g->AddDatumToInputStream("netdevs_1", dni::Datum(netdevs));

        g->AddDatumToInputStream("sip_count", dni::Datum(sip_count_ret));
}

int main()
{
        const std::string protoPath1 =
            "samples/ddos/realtime_with_sccm/pbtxt/graph1.pbtxt";
        const std::string protoPath2 =
            "samples/ddos/realtime_with_sccm/pbtxt/graph2.pbtxt";
        const std::string protoPath3 =
            "samples/ddos/realtime_with_sccm/pbtxt/graph3.pbtxt";

        size_t slot_size = (size_t) 20 * 1024 * 1024;   // one slot size, Bytes
        int slot_count = 100;                           // slot count

        // [notice] no need to copy data from share memory
        unsigned char* nic_data = 0;
        size_t rbsize = (size_t) (slot_size * slot_count + sizeof(ringbuffer_t));

        uint8_t* rb_buffer = buffer_create(rbsize);
        uint8_t* data_buffer = rb_buffer + sizeof(ringbuffer_t);

        ringbuffer_t* rb_header = (ringbuffer_t*) rb_buffer;
        rb_header->head = 0;
        rb_header->tail = 0;
        rb_header->size = (size_t) (slot_size * slot_count);

        size_t used;

        auto g1 = start_graph1(protoPath1);
        auto g2 = start_graph2(protoPath2);
        auto g3 = start_graph3(protoPath3);

        while (1)
        {
                // spdlog::info("------------- 1");
                // read data from ringbuffer
                if (rb_header->head <= rb_header->tail)
                {
                        used = rb_header->tail - rb_header->head;
                }
                else
                {
                        used = rb_header->size - (rb_header->head - rb_header->tail);
                }
                if (used < slot_size)
                {
                        // spdlog::info("no data in RB, continue...");
                        continue;
                }

                spdlog::info("--------------------- receive rb data!!!");

                nic_data = data_buffer + rb_header->head;

                rb_header->head = (rb_header->head + slot_size) % rb_header->size;

                dni::RBDataHeader rb_header;
                int offset;   // for jump to packets data

                // parse
                dni::parse_header(nic_data, rb_header, offset);

                // if countTotal is 0, stop
                if (rb_header.pkts_stats[0] == 0)
                {
                        spdlog::info("No packets");
                        continue;
                }

                spdlog::info("##################################");
                spdlog::info("countTotal: {}", rb_header.pkts_stats[0]);

                spdlog::info("host_nic_name: {}", rb_header.host_nic_name);
                spdlog::info("ts: {}", rb_header.ts);
                spdlog::info("pkts_stats: {}", rb_header.pkts_stats);
                spdlog::info("netdev_stats: {}", rb_header.netdev_stats);
                spdlog::info("speed: {}", rb_header.speed);
                spdlog::info("cur_cpu: {}", rb_header.cur_cpu);
                spdlog::info("resource_stats: {}", rb_header.resource_stats);
                spdlog::info("nic_ip: {:X}", rb_header.nic_ip);
                spdlog::info("mgr_ip: {:X}", rb_header.mgr_ip);
                spdlog::info("additional_stats: {}", rb_header.additional_stats);

                spdlog::info("##################################");

                anomalyResult anomaly_result = g1_anomaly_detect(g1, rb_header);

                if (anomaly_result.abnormal_number > 0)
                {
                        spdlog::info("Anomaly!!!");

                        // TODO: if need payload of packet
                        std::vector<std::unordered_map<std::string, uint32_t>> packets;

                        // if countTotal > 10000, sccm will apply 10000 packets
                        // else, will apply countTotal packets
                        uint32_t pkts_cnt =
                            (rb_header.pkts_stats[0] > 10000 ? 10000
                                                             : rb_header.pkts_stats[0]);
                        dni::parse_packets(nic_data + offset, pkts_cnt, packets);

                        spdlog::info("parsed_packets size: {}", packets.size());

                        std::unordered_map<uint32_t, int> sip_count_ret;
                        int attack_number = g2_attack_detect(g2, packets, sip_count_ret);
                        if (attack_number == 2)
                        {
                                spdlog::info("Confirmed Attack!!!");

                                std::string out = "dms_rules";

                                std::vector<uint32_t> all_known_ips = {};

                                std::vector<double_t> netdevs;
                                netdevs.push_back(anomaly_result.incrbytesRecv);
                                netdevs.push_back(anomaly_result.inMbps_score);
                                netdevs.push_back(anomaly_result.incrpacketRecv);
                                netdevs.push_back(anomaly_result.inKpps_score);
                                g3_inject_after(
                                    g3, packets, all_known_ips, netdevs,
                                    rb_header.host_nic_name, sip_count_ret);
                                g3->RunOnce();
                                g3->Wait();

                                auto ret = g3->GetResult<std::unordered_map<
                                    std::string, std::vector<dni::snding::DMSRule>>>(out);

                                spdlog::info("G3out {} result is: {}", out, ret.size());
                                for (auto&& pair : ret)
                                {
                                        spdlog::info(
                                            "host-nicname:{}\t , dms-rule size: "
                                            "{}\n",
                                            pair.first, pair.second.size());
                                        for (auto&& rule : pair.second)
                                        {
                                                spdlog::info("{}\n", rule);
                                        }
                                        // spdlog::info("-----------------------------------\n");
                                }

                                g3->ClearResult();
                        }
                        else if (attack_number == 1)
                        {
                                spdlog::info("Possible Attack.");
                        }
                        else
                        {
                                spdlog::info("No Attack.");
                        }
                }
                else
                {
                        spdlog::info("No Anomaly.\n\n");
                }
        }

        return 0;
}
