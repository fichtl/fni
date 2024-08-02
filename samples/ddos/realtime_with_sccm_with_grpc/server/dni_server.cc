#include "dni_server.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <fcntl.h>
#include <iostream>
#include <memory>
#include <pthread.h>
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
#include <vector>

#include "dni/framework/framework.h"
#include "dni/tasks/snding/snding_defines.h"
#include "fmt/format.h"
#include "samples/ddos/realtime_with_sccm_with_grpc/common/rb_data_mfr.h"
#include "samples/ddos/realtime_with_sccm_with_grpc/common/rb_data_parse.h"
#include "samples/ddos/realtime_with_sccm_with_grpc/common/snding_mfr_model.h"
#include "spdlog/spdlog.h"

const std::string protoPath1 = "./graph1.pbtxt";
const std::string protoPath2 = "./graph2.pbtxt";
const std::string protoPath3 = "./graph3.pbtxt";
const std::string protoPath4 = "./graph4.pbtxt";

// std::vector<uint32_t> all_known_ips;

// ServerWriter<GraphResponse>* g_writer;
// std::mutex g_writer_mutex;

#define SHM_KEY 0x1234

#define MFR_BATCH_SIZE 64

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
                SPDLOG_ERROR("shmget SHM_KEY");
                return NULL;
        }

        SPDLOG_INFO("shm_id: {}", shm_id);

        void* addr = shmat(shm_id, NULL, 0);
        if (addr == (void*) -1)
        {
                SPDLOG_ERROR("shmat SHM_KEY");
                return NULL;
        }

        return (uint8_t*) addr;
}

void buffer_free(uint8_t* buffer)
{
        if (shmdt((const void*) (buffer)) == -1)
        {
                SPDLOG_ERROR("shmdt buffer");
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
        g->ObserveOutputStream(out);

        std::string out1 = "netdev_abnormal";
        g->ObserveOutputStream(out1);

        std::string out2 = "resource_abnormal";
        g->ObserveOutputStream(out2);

        std::string netdev_inMbps = "netdev_inMbps";
        g->ObserveOutputStream(netdev_inMbps);

        std::string netdev_inKpps = "netdev_inKpps";
        g->ObserveOutputStream(netdev_inKpps);

        std::string out3 = "abnormal_res";
        g->ObserveOutputStream(out3);

        g->PrepareForRun();

        SPDLOG_INFO("start_graph1.");

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
        g->ObserveOutputStream(attack_result);

        std::string sip_count = "sip_count";
        g->ObserveOutputStream(sip_count);

        g->PrepareForRun();

        SPDLOG_INFO("start_graph2.");

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

        SPDLOG_INFO("start_graph3.");

        return g;
}

dni::Graph* start_graph4(const std::string& protoPath4)
{
        auto gc = dni::LoadTextprotoFile(protoPath4);
        if (!gc)
        {
                spdlog::error("invalid pbtxt config: {}", protoPath4);
                return nullptr;
        }

        dni::Graph* g = new dni::Graph(gc.value());

        std::string out = "ret";
        g->ObserveOutputStream(out);

        g->PrepareForRun();

        SPDLOG_INFO("start_graph4.");

        return g;
}

struct ingressData {
        double incrbytesRecv;
        double incrpacketRecv;
};

struct anomalyResult {
        int abnormal_number;
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
        // SPDLOG_INFO("Nout {} result is: {}", out, ret);

        auto ret1 = g->GetResult<double_t>(out1);
        // SPDLOG_INFO("Nout {} result is: {}", out1, ret1);

        auto ret2 = g->GetResult<double_t>(out2);
        // SPDLOG_INFO("Nout {} result is: {}", out2, ret2);

        auto ret3 = g->GetResult<int>(out3);
        // SPDLOG_INFO("G1out {} result is: {}", out3, ret3);

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

void g2_inject_after(dni::Graph* g, std::vector<std::vector<uint32_t>>& packets)
{
        g->AddDatumToInputStream("parsed_packets", dni::Datum(&packets));
}

int g2_attack_detect(
    dni::Graph* g, std::vector<std::vector<uint32_t>>& packets,
    std::unordered_map<uint32_t, int>& sip_count_ret)
{
        std::string attack_result = "attack_res";
        std::string sip_count = "sip_count";

        g2_inject_after(g, packets);
        g->RunOnce();
        g->Wait();

        auto attack_number = g->GetResult<double_t>(attack_result);
        // SPDLOG_INFO("G2out {} result is: {}", attack_result, attack_number);

        sip_count_ret = g->GetResult<std::unordered_map<uint32_t, int>>(sip_count);
        // SPDLOG_INFO("G2out {} result size is: {}", sip_count, sip_count_ret.size());

        g->ClearResult();

        return attack_number;
}

void g3_inject_after(
    dni::Graph* g, std::vector<std::vector<uint32_t>>& packets,
    std::vector<uint32_t> all_known_ips, std::vector<double> netdevs,
    std::string host_nic_name, std::unordered_map<uint32_t, int>& sip_count_ret)
{
        g->AddDatumToInputStream("parsed_packets", dni::Datum(&packets));

        g->AddDatumToInputStream("all_known_ips", dni::Datum(all_known_ips));

        g->AddDatumToInputStream("host_nic_name", dni::Datum(std::string(host_nic_name)));

        g->AddDatumToInputStream("netdevs_1", dni::Datum(netdevs));

        g->AddDatumToInputStream("sip_count", dni::Datum(&sip_count_ret));
}

void userPrepareInputData4(
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

void inject_after4(dni::Graph* g, float* model_np_list, int64_t total)
{
        std::vector<std::vector<float>*> input_data;
        userPrepareInputData4(input_data, model_np_list, total);   // user call

        g->AddDatumToInputStream("data", dni::Datum(input_data));
}

class spsc {
        std::vector<unsigned char*> data;
        std::atomic<int> head{0}, tail{0};
        int Cap_;

public:
        spsc(int Cap)
        {
                Cap_ = Cap;
                data.resize(Cap_);
        }
        spsc(const spsc&) = delete;
        spsc& operator=(const spsc&) = delete;
        spsc& operator=(const spsc&) volatile = delete;

        void push(unsigned char* d)
        {
                int t = tail.load(std::memory_order_relaxed);
                data[t] = d;
                tail.store((t + 1) % Cap_, std::memory_order_release);
        }

        bool pop(unsigned char*& val)
        {
                int h = head.load(std::memory_order_relaxed);
                if (h == tail.load(std::memory_order_acquire))
                {
                        return false;
                }

                val = data[h];
                head.store((h + 1) % Cap_, std::memory_order_release);

                return true;
        }
};

struct graph_t {
        int index = -1;
        dni::Graph* g1;
        dni::Graph* g2;
        dni::Graph* g3;
        dni::Graph* g4;

        // std::list<unsigned char*> slots;
        // std::mutex mutex_lock;

        spsc* ptrs_rb;
};

void* calc_graph(
    graph_t* g, std::vector<uint32_t>& all_known_ips, ServerWriter<GraphResponse>* writer,
    std::mutex* writer_mutex, uint32_t count_total_threshold)
{
        if (!g || !writer || !writer_mutex)
        {
                return nullptr;
        }

        int i = g->index;
        SPDLOG_INFO("calc_graph {} start", i);
        unsigned char* nic_data;
        std::vector<std::string> mfr_bytes_list;
        float* model_np_list =
            (float*) malloc(MFR_BATCH_SIZE * 1 * 40 * 40 * sizeof(float));
        memset(model_np_list, 0, MFR_BATCH_SIZE * 1 * 40 * 40 * sizeof(float));
        while (1)
        {
                if (!g->ptrs_rb->pop(nic_data))
                {
                        // SPDLOG_INFO("Wait for packets");
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        continue;
                }

                dni::RBDataHeader rb_header;
                int offset;   // for jump to packets data

                // parse
                dni::parse_header(nic_data, rb_header, offset);
                SPDLOG_INFO("countTotal is: {}", rb_header.pkts_stats[0]);

                GraphResponse response;
                GraphCalcResult* result;
                result = response.add_results();

                // if countTotal is less than count_total_threshold,
                // no need to analyze by dni graph, just response
                if (rb_header.pkts_stats[0] <= count_total_threshold)
                {
                        SPDLOG_INFO("Less packets to analyze");
                        result->set_ts(Utils::now());
                        result->set_type(GraphResultType::GraphCalcOK);
                        result->set_host_nic_name(rb_header.host_nic_name);
                        result->set_abnormal_type(0);
                        result->set_attack_type(0);

                        GraphDMSRule* rule = result->add_rules();
                        rule->set_packets_ts(rb_header.ts);

                        writer_mutex->lock();
                        if (!writer->Write(response))
                        {
                                SPDLOG_WARN("the stream has been closed.");
                                writer_mutex->unlock();

                                break;
                        }

                        writer_mutex->unlock();

                        continue;
                }

                // ######
                // int64_t now = (((int64_t) Utils::now()) / (int64_t) 1000000000);
                // int64_t sccm_ts = rb_header.ts;

                // SPDLOG_INFO("in process..., now: {}, sccm ts: {}", now, sccm_ts);

                // if (now - sccm_ts > 10)
                // {
                //         SPDLOG_WARN(
                //             "@@@@ too slow    in process..., {}, {}, {}", now, sccm_ts,
                //             now - sccm_ts);
                // }
                // ######

                // SPDLOG_INFO("##################################");
                // SPDLOG_INFO("countTotal: {}", rb_header.pkts_stats[0]);

                // SPDLOG_INFO("host_nic_name: {}", rb_header.host_nic_name);
                // SPDLOG_INFO("ts: {}", rb_header.ts);
                // SPDLOG_INFO("pkts_stats: {}", rb_header.pkts_stats);
                // SPDLOG_INFO("netdev_stats: {}", rb_header.netdev_stats);
                // SPDLOG_INFO("speed: {}", rb_header.speed);
                // SPDLOG_INFO("cur_cpu: {}", rb_header.cur_cpu);
                // SPDLOG_INFO("resource_stats: {}", rb_header.resource_stats);
                // SPDLOG_INFO("nic_ip: {:X}", rb_header.nic_ip);
                // SPDLOG_INFO("mgr_ip: {:X}", rb_header.mgr_ip);
                // SPDLOG_INFO("additional_stats: {}", rb_header.additional_stats);

                // SPDLOG_INFO("##################################");

                // std::string str_ret = "";
                // str_ret +=
                //     ("# single_nic_analysis inMbps: *" +
                //      std::to_string(rb_header.netdev_stats[0]) + "*\n");
                // str_ret +=
                //     ("# single_nic_analysis outMbps: *" +
                //      std::to_string(rb_header.netdev_stats[2]) + "*\n");
                // str_ret +=
                //     ("# single_nic_analysis dmsDropMbps: *" +
                //      std::to_string(
                //          rb_header.additional_stats[4] * 8.0 / 1000.0 / 1000.0) +
                //      "*\n");
                // str_ret +=
                //     ("# single_nic_analysis speedStr: *" +
                //      std::to_string(rb_header.speed) + "Mb/s*\n");
                // str_ret +=
                //     ("# single_nic_analysis cur_cpu: *" +
                //      std::to_string(rb_header.cur_cpu) + "*\n");
                // str_ret +=
                //     ("# single_nic_analysis tag: *" + std::to_string(rb_header.ts) +
                //      "*\n");

                // timestamp of server send resp
                result->set_ts(Utils::now());
                result->set_type(GraphResultType::GraphCalcOK);
                result->set_host_nic_name(rb_header.host_nic_name);
                result->set_inmbps(rb_header.netdev_stats[0]);
                result->set_outmbps(rb_header.netdev_stats[2]);
                result->set_dmsdropmbps(
                    (double) rb_header.additional_stats[4] * 8.0 / 1000.0 / 1000.0);
                result->set_speed(rb_header.speed);
                result->set_cur_cpu(rb_header.cur_cpu);

                auto a = Utils::now();

                anomalyResult anomaly_result = g1_anomaly_detect(g->g1, rb_header);

                result->set_abnormal_type(anomaly_result.abnormal_number);

                if (anomaly_result.abnormal_number > 0)
                {
                        // SPDLOG_INFO("Anomaly!!!");

                        // TODO: if need payload of packet
                        // 0:SIP, 1:SPort, 2:DPort, 3:Protocol, 4:Length, 5:DIP
                        std::vector<std::vector<uint32_t>> packets;

                        // if countTotal > 10000, sccm will apply 10000 packets
                        // else, will apply countTotal packets
                        uint32_t pkts_cnt =
                            (rb_header.pkts_stats[0] > 10000 ? 10000
                                                             : rb_header.pkts_stats[0]);

                        packets.reserve(pkts_cnt);
                        dni::parse_packets(nic_data + offset, pkts_cnt, packets);

                        SPDLOG_INFO(
                            "want host nic name: {}, {}, {}, {}", rb_header.host_nic_name,
                            rb_header.ts, rb_header.pkts_stats[0], packets.size());

                        if (packets.size() == 0)
                        {
                                SPDLOG_WARN(
                                    "strange, packets size is 0, {}, {}, {}",
                                    rb_header.host_nic_name, rb_header.ts,
                                    rb_header.pkts_stats[0]);

                                continue;
                        }

                        // SPDLOG_INFO("parsed_packets size: {}", packets.size());

                        std::unordered_map<uint32_t, int> sip_count_ret;
                        int attack_number =
                            g2_attack_detect(g->g2, packets, sip_count_ret);

                        result->set_attack_type(attack_number);
                        if (attack_number == 2)
                        {
                                // SPDLOG_INFO("Confirmed Attack!!!");

                                std::string out = "dms_rules";

                                std::vector<double_t> netdevs;
                                netdevs.push_back(anomaly_result.incrbytesRecv);
                                netdevs.push_back(anomaly_result.inMbps_score);
                                netdevs.push_back(anomaly_result.incrpacketRecv);
                                netdevs.push_back(anomaly_result.inKpps_score);
                                g3_inject_after(
                                    g->g3, packets, all_known_ips, netdevs,
                                    rb_header.host_nic_name, sip_count_ret);
                                g->g3->RunOnce();
                                g->g3->Wait();

                                auto ret = g->g3->GetResult<std::unordered_map<
                                    std::string, std::vector<dni::snding::DMSRule>>>(out);

                                auto b = Utils::now();
                                auto c = b - a;
                                SPDLOG_WARN(
                                    "############ graph {} cost {} ms #############", i,
                                    c / 1000000);

                                // SPDLOG_INFO("G3out {} result is: {}", out,
                                // ret.size());

                                // str_ret += ("# single_nic_analysis result: *Confirmed "
                                //             "Attack*\n");

                                result->set_detect_result("Confirmed Attack");

                                // std::string str_rules = "";
                                for (auto&& p : ret)
                                {
                                        // str_rules += ("host-nicname:" + p.first + ":");
                                        for (auto&& d : p.second)
                                        {
                                                // str_rules +=
                                                //     ("[" + fmt::to_string(d) + "]\n");

                                                GraphDMSRule* rule = result->add_rules();
                                                rule->set_hostnicsign(p.first);

                                                rule->set_srcip(d.srcIP.ip);
                                                rule->set_srcip_len(d.srcIP.len);
                                                rule->set_dstip(d.dstIP.ip);
                                                rule->set_dstip_len(d.dstIP.len);

                                                rule->set_sport(d.sPort);
                                                rule->set_dport(d.dPort);
                                                rule->set_length(d.length);
                                                rule->set_protocol(d.protocol);

                                                rule->set_action(d.action);
                                                rule->set_limitmode(d.limitMode);
                                                rule->set_limitmaxvalue(d.limitMaxValue);

                                                // timestamp of packets from sccm
                                                rule->set_packets_ts(rb_header.ts);
                                        }
                                }

                                // str_ret +=
                                //     ("# single_nic_analysis detail: *{" + str_rules +
                                //      "}*");

                                g->g3->ClearResult();
                        }
                        else if (attack_number == 1)
                        {
                                SPDLOG_INFO("Possible Attack.");

                                // str_ret +=
                                //     ("# single_nic_analysis result: *Possible
                                //     Attack*\n");
                                dni::get_mfr_bytes(
                                    mfr_bytes_list, nic_data + offset, pkts_cnt,
                                    MFR_BATCH_SIZE);

                                dni::get_model_input(mfr_bytes_list, model_np_list);
                                inject_after4(
                                    g->g4, model_np_list,
                                    (int64_t) MFR_BATCH_SIZE * 1 * 40 * 40);

                                g->g4->RunOnce();

                                g->g4->Wait();

                                auto ret = g->g4->GetResult<
                                    std::vector<std::vector<std::vector<float_t>>>>(
                                    "ret");

                                // spdlog::info("Gout {} result is: {}", out, ret);
                                int label_relation_index = -1;
                                auto predicted_class = dni::get_label_from_infer_ret(
                                    ret, label_relation_index);
                                spdlog::info(
                                    "label_relation_index: {}, Predicted class: {}",
                                    label_relation_index, predicted_class);

                                result->set_detect_result("Possible Attack");
                                result->set_label_relation_index(label_relation_index);

                                GraphDMSRule* rule = result->add_rules();
                                rule->set_packets_ts(rb_header.ts);

                                mfr_bytes_list.clear();
                                memset(
                                    model_np_list, 0,
                                    MFR_BATCH_SIZE * 1 * 40 * 40 * sizeof(float));
                        }
                        else
                        {
                                SPDLOG_INFO("No Attack.");

                                // str_ret +=
                                //     ("# single_nic_analysis result: *No Attack*\n");

                                result->set_detect_result("No Attack");

                                GraphDMSRule* rule = result->add_rules();
                                rule->set_packets_ts(rb_header.ts);
                        }
                }
                else
                {
                        SPDLOG_INFO("No Anomaly.\n\n");

                        // str_ret += ("# single_nic_analysis result: *No Anomaly*\n");

                        result->set_detect_result("No Anomaly");

                        GraphDMSRule* rule = result->add_rules();
                        rule->set_packets_ts(rb_header.ts);
                }

                // SPDLOG_INFO("graph: {}, {}", g->index, str_ret);

                writer_mutex->lock();
                if (!writer->Write(response))
                {
                        SPDLOG_WARN("the stream has been closed.");
                        writer_mutex->unlock();

                        break;
                }

                writer_mutex->unlock();
        }

        g->g1->Cancel();
        g->g2->Cancel();
        g->g3->Cancel();
        g->g4->Cancel();

        g->g1->Finish();
        g->g2->Finish();
        g->g3->Finish();
        g->g4->Finish();

        free(model_np_list);
        model_np_list = 0;

        return nullptr;
}

void DNIServiceImpl::printHeaders(const ServerContext* context)
{
        const multimap<grpc::string_ref, grpc::string_ref>& metadata =
            context->client_metadata();
        for (const auto& iter : metadata)
        {
                const grpc::string_ref& key = iter.first;
                const grpc::string_ref& value = iter.second;
                // SPDLOG_DEBUG(
                //     "->H {}: {}", std::string(key.begin(), key.end()),
                //     std::string(value.begin(), value.end()));
        }
}

void DNIServiceImpl::propagateHeaders(
    const ServerContext* context, grpc::ClientContext& c)
{
        const multimap<grpc::string_ref, grpc::string_ref>& metadata =
            context->client_metadata();
        for (const auto& iter : metadata)
        {
                const grpc::string_ref& key = iter.first;
                const grpc::string_ref& value = iter.second;
                // SPDLOG_DEBUG(
                //     "->H {}: {}", std::string(key.begin(), key.end()),
                //     std::string(value.begin(), value.end()));
        }
}

void DNIServiceImpl::setChannel(const std::shared_ptr<Channel>& channel)
{
        if (channel != nullptr)
        {
                client = DNIService::NewStub(channel);
        }
}

void start_graph(
    std::vector<graph_t*>& concurrent_graphs, std::vector<std::thread>& threads, int cnt,
    std::vector<uint32_t>& all_known_ips, ServerWriter<GraphResponse>* writer,
    std::mutex* writer_mutex, uint32_t count_total_threshold)
{
        SPDLOG_WARN("cnt {}", cnt);

        for (size_t i = 0; i < cnt; i++)
        {
                graph_t* g = new graph_t();

                g->index = i;
                SPDLOG_INFO("g->index {}", g->index);

                g->g1 = start_graph1(protoPath1);
                g->g2 = start_graph2(protoPath2);
                g->g3 = start_graph3(protoPath3);
                g->g4 = start_graph4(protoPath4);

                g->ptrs_rb = new spsc(100);

                concurrent_graphs.push_back(g);

                threads.push_back(std::thread([&]() {
                        calc_graph(
                            g, std::ref(all_known_ips), writer, writer_mutex,
                            count_total_threshold);
                }));

                std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
}

void* read_rb(uint8_t* rb_buffer, int graph_cnt, std::vector<graph_t*>& concurrent_graphs)
{
        size_t slot_size = (size_t) 20 * 1024 * 1024;   // one slot size, Bytes
        int slot_count = 100;                           // slot count

        // [notice] no need to copy data from share memory
        unsigned char* nic_data = 0;
        size_t rbsize = (size_t) (slot_size * slot_count + sizeof(ringbuffer_t));

        rb_buffer = buffer_create(rbsize);
        uint8_t* data_buffer = rb_buffer + sizeof(ringbuffer_t);

        ringbuffer_t* rb_header = (ringbuffer_t*) rb_buffer;
        rb_header->head = 0;
        rb_header->tail = 0;
        rb_header->size = (size_t) (slot_size * slot_count);

        size_t used;

        int graph_index = 0;

        SPDLOG_INFO("read_rb start");

        while (1)
        {
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
                        // SPDLOG_INFO("no data in RB, continue...");
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        continue;
                }

                nic_data = data_buffer + rb_header->head;

                rb_header->head = (rb_header->head + slot_size) % rb_header->size;

                auto& g = concurrent_graphs[graph_index];

                // ######
                unsigned char* slot = nic_data;
                uint16_t nic_name_len = *((uint16_t*) slot);
                slot += 2;
                slot += nic_name_len;

                // ######
                // int64_t now = (((int64_t) Utils::now()) / (int64_t) 1000000000);
                // int64_t sccm_ts = *((uint64_t*) slot);

                // SPDLOG_INFO(
                //     "receive rb data, push to graph_index: {}, now: {}, sccm ts: {}",
                //     graph_index, now, sccm_ts);

                // if (now - sccm_ts > 10)
                // {
                //         SPDLOG_WARN(
                //             "@@@@ too slow, {}, {}, {}", now, sccm_ts, now - sccm_ts);
                // }

                // g->mutex_lock.lock();
                // g->slots.push_back(nic_data);
                // g->mutex_lock.unlock();

                g->ptrs_rb->push(nic_data);

                graph_index = (graph_index + 1) % graph_cnt;
        }
}

Status DNIServiceImpl::CalculateGraph(
    ServerContext* context, const GraphRequest* request,
    ServerWriter<GraphResponse>* writer)
{
        SPDLOG_INFO("CalculateGraph Enter");
        // printHeaders(context);
        if (client != nullptr)
        {
                grpc::ClientContext c;
                propagateHeaders(context, c);
                GraphResponse graphResponse;
                const std::unique_ptr<::grpc::ClientReader<GraphResponse>>& response(
                    client->CalculateGraph(&c, *request));
                while (response->Read(&graphResponse))
                {
                        writer->Write(graphResponse);
                }

                return Status::OK;
        }

        uint32_t count_total_threshold = request->count_total_threshold();
        SPDLOG_WARN("count_total_threshold: {}", count_total_threshold);

        std::vector<uint32_t> all_known_ips;
        for (auto&& ip : request->all_ips())
        {
                all_known_ips.push_back(ip);
        }
        std::sort(all_known_ips.begin(), all_known_ips.end());
        SPDLOG_INFO("all_known_ips size: {}", all_known_ips.size());

        std::mutex writer_mutex;

        auto all_nic_number = request->all_nic_number();
        int concur_cnt = std::thread::hardware_concurrency();
        SPDLOG_INFO("all_nic_number: {}, concur_cnt: {}", all_nic_number, concur_cnt);

        std::vector<graph_t*> concurrent_graphs;
        std::vector<std::thread> threads;
        // int graph_cnt =
        //     ((all_nic_number / 3 + 1) > (concur_cnt / 3 + 1) ? (concur_cnt / 3 + 1)
        //                                                      : (all_nic_number / 3 +
        //                                                      1));

        // int graph_cnt =
        //     ((all_nic_number) > (concur_cnt / 2) ? (concur_cnt / 2) :
        //     (all_nic_number));
        int graph_cnt = 1;

        start_graph(
            concurrent_graphs, threads, graph_cnt, all_known_ips, writer, &writer_mutex,
            count_total_threshold);

        uint8_t* rb_buffer = 0;
        auto rb_thread = std::thread([&]() {
                read_rb(std::ref(rb_buffer), graph_cnt, std::ref(concurrent_graphs));
        });

        for (auto& t : threads)
        {
                t.join();
        }
        SPDLOG_WARN("graph threads JOIN");
        for (auto&& g : concurrent_graphs)
        {
                delete g->g1;
                delete g->g2;
                delete g->g3;
                delete g->g4;
                delete g->ptrs_rb;
                delete g;
        }

        SPDLOG_INFO("rb_thread.joinable() = {}", rb_thread.joinable());
        pthread_cancel(rb_thread.native_handle());
        SPDLOG_INFO("rb_thread.joinable() = {}", rb_thread.joinable());
        rb_thread.join();
        SPDLOG_WARN("rb_thread JOIN");

        // buffer_free(rb_buffer);
        // rb_buffer = 0;

        all_known_ips.clear();

        SPDLOG_WARN("CalculateGraph Exit");

        return Status::OK;
}
