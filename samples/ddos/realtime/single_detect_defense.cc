#include "influxdb.hpp"
#include <iostream>
#include <string>
#include <json/json.h>

#include <chrono>
#include <thread>
#include "dni/framework/framework.h"
#include "spdlog/spdlog.h"
#include "dni/tasks/snding/snding_defines.h"


// get nic(packet/netdev) stat data
Json::Value getNicStat(const std::string& tableName,
        const std::string& hostName, const std::string& nicName) {
     try {
        // 连接 InfluxDB 数据库
        influxdb_cpp::server_info insense("172.17.17.27", 8086, "snding", "", "");
        std::string resp_nic;
        std::string nic_sql = "select * from " + tableName +
                                " where time >= now() - 3s and host = '" + hostName +
                                "' and netif = '" + nicName +
                                "' order by time desc limit 1";
        std::cout << nic_sql << std::endl;
        int success = influxdb_cpp::query(resp_nic, nic_sql, insense);
        // std::cout << success << std::endl;
        if (success == 0) {
            // std::cout << "Query successful!" << std::endl;
            std::cout << resp_nic << std::endl;
        } else {
            std::cout << "Query failed." << std::endl;
        };
        // 创建 JSON 解析器
        Json::CharReaderBuilder readerBuilder;
        Json::Value root;
        std::string errs;
        // 解析 JSON 字符串
        std::istringstream s(resp_nic);
        if (!Json::parseFromStream(readerBuilder, s, &root, &errs)) {
            std::cerr << "Failed to parse JSON: " << errs << std::endl;
        }
        const Json::Value& results = root["results"];
        const Json::Value& series = results[0]["series"];
        const Json::Value& values = series[0]["values"];
        const Json::Value& nicStatValue = values[0];
        // 返回结果
        return nicStatValue;
    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
    }
}


// get host(resource) stat data
Json::Value getHostStat(const std::string& tableName,
        const std::string& hostName) {
     try {
        // 连接 InfluxDB 数据库
        influxdb_cpp::server_info insense("172.17.17.27", 8086, "snding", "", "");
        std::string resp_host;
        std::string host_sql = "select * from " + tableName +
                                " where time >= now() - 3s and host = '" + hostName +
                                "' order by time desc limit 1";
        std::cout << host_sql << std::endl;
        int success = influxdb_cpp::query(resp_host, host_sql, insense);
        // std::cout << success << std::endl;
        if (success == 0) {
            // std::cout << "Query successful!" << std::endl;
            std::cout << resp_host << std::endl;
        } else {
            std::cout << "Query failed." << std::endl;
        };
        // 创建 JSON 解析器
        Json::CharReaderBuilder readerBuilder;
        Json::Value root;
        std::string errs;
        // 解析 JSON 字符串
        std::istringstream s(resp_host);
        if (!Json::parseFromStream(readerBuilder, s, &root, &errs)) {
            std::cerr << "Failed to parse JSON: " << errs << std::endl;
        }
        const Json::Value& results = root["results"];
        const Json::Value& series = results[0]["series"];
        const Json::Value& values = series[0]["values"];
        const Json::Value& hostStatValue = values[0];
        // 返回结果
        return hostStatValue;
    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
    }
}


struct ingressData {
    std::string pcapNameStr;
    double  incrbytesRecv;
    double  incrpacketRecv;
};

ingressData inject_after(dni::Graph* g, int after, int n, int interval,
    const std::string& hostName, const std::string& nicName,
    const std::string& tableName, const std::string& tableName1,
    const std::string& tableName2)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(after));

    // std::string hostName = "ddos21";
    // std::string nicName = "enp1s0f1";
    // std::string tableName = "packet";
    // std::string tableName1 = "netdev";
    // std::string tableName2 = "resource";

    // 1. packet stat
    Json::Value packetStatValue = getNicStat(tableName, hostName, nicName);
    // std::cout << packetStatValue.toStyledString() << std::endl;
    // Extract the desired values based on their indices in the 'columns' array
    std::string countTotalStr = std::to_string(packetStatValue[2].asInt());
    std::string countType_dstIPStr = std::to_string(packetStatValue[3].asInt());
    std::string countType_dstPortTcpStr = std::to_string(packetStatValue[4].asInt());
    std::string countType_dstPortUdpStr = std::to_string(packetStatValue[5].asInt());
    std::string countType_lengthStr = std::to_string(packetStatValue[6].asInt());
    std::string countType_protocolStr = std::to_string(packetStatValue[7].asInt());
    std::string countType_srcIPStr = std::to_string(packetStatValue[8].asInt());
    std::string countType_srcPortTcpStr = std::to_string(packetStatValue[9].asInt());
    std::string countType_srcPortUdpStr = std::to_string(packetStatValue[10].asInt());
    std::string pcapNameStr = packetStatValue[16].asString();
    // Convert string to double
    double countTotal = std::stod(countTotalStr);
    double countType_dstIP = std::stod(countType_dstIPStr);
    double countType_dstPortTcp = std::stod(countType_dstPortTcpStr);
    double countType_dstPortUdp = std::stod(countType_dstPortUdpStr);
    double countType_length = std::stod(countType_lengthStr);
    double countType_protocol = std::stod(countType_protocolStr);
    double countType_srcIP = std::stod(countType_srcIPStr);
    double countType_srcPortTcp = std::stod(countType_srcPortTcpStr);
    double countType_srcPortUdp = std::stod(countType_srcPortUdpStr);
    // cond data
    std::vector<double> cond_countType_length = {countTotal};
    std::vector<double> cond_countType_srcIP = {countTotal};
    std::vector<double> cond_countType_dstIP = {countTotal};
    std::vector<double> cond_countType_protocol = {countTotal};
    std::vector<double> cond_countType_srcPortTcp = {countTotal};
    std::vector<double> cond_countType_dstPortTcp = {countTotal};
    std::vector<double> cond_countType_srcPortUdp = {countTotal};
    std::vector<double> cond_countType_dstPortUdp = {countTotal};
    // Output the extracted values
    std::cout << "countTotal: " << countTotal << std::endl;
    std::cout << "countType_dstIP: " << countType_dstIP << std::endl;
    std::cout << "countType_dstPortTcp: " << countType_dstPortTcp << std::endl;
    std::cout << "countType_dstPortUdp: " << countType_dstPortUdp << std::endl;
    std::cout << "countType_length: " << countType_length << std::endl;
    std::cout << "countType_protocol: " << countType_protocol << std::endl;
    std::cout << "countType_srcIP: " << countType_srcIP << std::endl;
    std::cout << "countType_srcPortTcp: " << countType_srcPortTcp << std::endl;
    std::cout << "countType_srcPortUdp: " << countType_srcPortUdp << std::endl;
    std::cout << "pcapName: " << pcapNameStr << std::endl;
    std::cout << "cond_countTotal: " << cond_countType_dstIP[0] << std::endl;

    // 2. netdev stat
    Json::Value netdevStatValue = getNicStat(tableName1, hostName, nicName);
    // std::cout << netdevStatValue.toStyledString() << std::endl;
    // Extract the desired values based on their indices in the 'columns' array
    std::string incrbytesRecvStr = std::to_string(netdevStatValue[6].asInt());
    std::string incrpacketRecvStr = std::to_string(netdevStatValue[10].asInt());
    std::string incrbytesSentStr = std::to_string(netdevStatValue[7].asInt());
    std::string incrpacketSentStr = std::to_string(netdevStatValue[11].asInt());
    // Convert string to double
    double incrbytesRecv = std::stod(incrbytesRecvStr);
    double incrpacketRecv = std::stod(incrpacketRecvStr);
    double incrbytesSent = std::stod(incrbytesSentStr);
    double incrpacketSent = std::stod(incrpacketSentStr);
    // unit conversion
    double inMbps = incrbytesRecv * 8 / (1000 * 1000);
    double inKpps = incrpacketRecv / 1000;
    double outMbps = incrbytesSent * 8 / (1000 * 1000);
    double outKpps = incrpacketSent / 1000;
    // Output the extracted values
    std::cout << "inMbps: " << inMbps << std::endl;
    std::cout << "inKpps: " << inKpps << std::endl;
    std::cout << "outMbps: " << outMbps << std::endl;
    std::cout << "outKpps: " << outKpps << std::endl;

    // 3. resource stat
    Json::Value resourceStatValue = getHostStat(tableName2, hostName);
    // std::cout << resourceStatValue.toStyledString() << std::endl;
    // Extract the desired values based on their indices in the 'columns' array
    std::string cur_cpuStr = std::to_string(resourceStatValue[1].asInt());
    std::string incr_mem_swapStr = std::to_string(resourceStatValue[8].asInt());
    std::string incr_mem_virtualStr = std::to_string(resourceStatValue[9].asInt());
    std::string incr_tcpconn_fullStr = std::to_string(resourceStatValue[10].asInt());
    std::string incr_tcpconn_semiStr = std::to_string(resourceStatValue[11].asInt());
    std::string incr_tcpconn_totalStr = std::to_string(resourceStatValue[12].asInt());
    // Convert string to double
    double cur_cpu = std::stod(cur_cpuStr);
    double incr_mem_swap = std::stod(incr_mem_swapStr);
    double incr_mem_virtual = std::stod(incr_mem_virtualStr);
    double incr_tcpconn_full = std::stod(incr_tcpconn_fullStr);
    double incr_tcpconn_semi = std::stod(incr_tcpconn_semiStr);
    double incr_tcpconn_total = std::stod(incr_tcpconn_totalStr);
    // Output the extracted values
    std::cout << "cur_cpu: " << cur_cpu << std::endl;
    std::cout << "incr_mem_swap: " << incr_mem_swap << std::endl;
    std::cout << "incr_mem_virtual: " << incr_mem_virtual << std::endl;
    std::cout << "incr_tcpconn_full: " << incr_tcpconn_full << std::endl;
    std::cout << "incr_tcpconn_semi: " << incr_tcpconn_semi << std::endl;
    std::cout << "incr_tcpconn_total: " << incr_tcpconn_total << std::endl;

    for (int i = 0; i < n; i++)
        {
                // packet: countTotal
                g->AddDatumToInputStream("countTotal", dni::Datum(countTotal));
                // packet: countType_length
                g->AddDatumToInputStream(
                    "countType_length", dni::Datum(countType_length));
                g->AddDatumToInputStream(
                    "cond_countType_length", dni::Datum(cond_countType_length));
                // packet: countType_srcIP
                g->AddDatumToInputStream("countType_srcIP", dni::Datum(countType_srcIP));
                g->AddDatumToInputStream(
                    "cond_countType_srcIP", dni::Datum(cond_countType_srcIP));
                // packet: countType_dstIP
                g->AddDatumToInputStream("countType_dstIP", dni::Datum(countType_dstIP));
                g->AddDatumToInputStream(
                    "cond_countType_dstIP", dni::Datum(cond_countType_dstIP));
                // packet: countType_protocol
                g->AddDatumToInputStream(
                    "countType_protocol", dni::Datum(countType_protocol));
                g->AddDatumToInputStream(
                    "cond_countType_protocol", dni::Datum(cond_countType_protocol));
                // packet: countType_srcPortTcp
                g->AddDatumToInputStream(
                    "countType_srcPortTcp", dni::Datum(countType_srcPortTcp));
                g->AddDatumToInputStream(
                    "cond_countType_srcPortTcp", dni::Datum(cond_countType_srcPortTcp));
                // packet: countType_dstPortTcp
                g->AddDatumToInputStream(
                    "countType_dstPortTcp", dni::Datum(countType_dstPortTcp));
                g->AddDatumToInputStream(
                    "cond_countType_dstPortTcp", dni::Datum(cond_countType_dstPortTcp));
                // packet: countType_srcPortUdp
                g->AddDatumToInputStream(
                    "countType_srcPortUdp", dni::Datum(countType_srcPortUdp));
                g->AddDatumToInputStream(
                    "cond_countType_srcPortUdp", dni::Datum(cond_countType_srcPortUdp));
                // packet: countType_dstPortUdp
                g->AddDatumToInputStream(
                    "countType_dstPortUdp", dni::Datum(countType_dstPortUdp));
                g->AddDatumToInputStream(
                    "cond_countType_dstPortUdp", dni::Datum(cond_countType_dstPortUdp));

                // netdev: inMbps
                g->AddDatumToInputStream("inMbps", dni::Datum(inMbps));
                // netdev: inKpps
                g->AddDatumToInputStream("inKpps", dni::Datum(inKpps));
                // netdev: outMbps
                g->AddDatumToInputStream("outMbps", dni::Datum(outMbps));
                // netdev: outKpps
                g->AddDatumToInputStream("outKpps", dni::Datum(outKpps));

                // resource: cur_cpu
                g->AddDatumToInputStream("cur_cpu", dni::Datum(cur_cpu));
                // resource: incr_mem_swap
                g->AddDatumToInputStream("incr_mem_swap", dni::Datum(incr_mem_swap));
                // resource: incr_mem_virtual
                g->AddDatumToInputStream(
                    "incr_mem_virtual", dni::Datum(incr_mem_virtual));
                // resource: incr_mem_full
                g->AddDatumToInputStream("incr_tcpconn_full", dni::Datum(incr_tcpconn_full));
                // resource: incr_tcpconn_semi
                g->AddDatumToInputStream(
                    "incr_tcpconn_semi", dni::Datum(incr_tcpconn_semi));
                // resource: incr_tcpconn_total
                g->AddDatumToInputStream(
                    "incr_tcpconn_total", dni::Datum(incr_tcpconn_total));

                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        }
    ingressData ingress_data;
    ingress_data.pcapNameStr = pcapNameStr;
    ingress_data.incrbytesRecv = incrbytesRecv;
    ingress_data.incrpacketRecv = incrpacketRecv;
    return ingress_data;

}

struct anomalyResult {
    int abnormal_number;
    std::string pcapName;
    double  incrbytesRecv;
    double  incrpacketRecv;
    double  inMbps_score;
    double  inKpps_score;
};

anomalyResult anomaly_detect(std::string hostName, std::string nicName,
        std::string tableName, std::string tableName1,
        std::string tableName2, std::string protoPath1) {
    auto gc = dni::LoadTextprotoFile(protoPath1);
    if (!gc)
    {
            spdlog::error("invalid pbtxt config: {}", protoPath1);
            // return -1;
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
    ingressData ingress_data = inject_after(g, 0, 1, 0, hostName, nicName, tableName, tableName1, tableName2);
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
    spdlog::info("G1out pcapName result is: {}", ingress_data.pcapNameStr);

    auto inMbps_score = g->GetResult<double_t>(netdev_inMbps);
    auto inKpps_score = g->GetResult<double_t>(netdev_inKpps);
    g->Finish();

    anomalyResult anomaly_result;
    anomaly_result.abnormal_number = ret3;
    anomaly_result.pcapName = ingress_data.pcapNameStr;
    anomaly_result.incrbytesRecv = ingress_data.incrbytesRecv;
    anomaly_result.inMbps_score = inMbps_score;
    anomaly_result.inKpps_score = inKpps_score;
    anomaly_result.incrpacketRecv = ingress_data.incrpacketRecv;
    return anomaly_result;
}

void inject_after1(
    dni::Graph* g, int after, int n, int interval, std::string& pcapNamePath)
{
        std::this_thread::sleep_for(std::chrono::milliseconds(after));

        for (int i = 0; i < n; i++)
        {
                SPDLOG_DEBUG(
                    "send Datum({}) to graph g({:p})",
                    dni::Datum(std::string(pcapNamePath)), fmt::ptr(g));

                // abnormal_nic_pcap_parser
                g->AddDatumToInputStream(
                    "pcapPath", dni::Datum(std::string(pcapNamePath)));

                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        }
}

int attack_detect(std::string protoPath2, std::string pcapNamePath){
    auto gc = dni::LoadTextprotoFile(protoPath2);
    if (!gc)
    {
            spdlog::error("invalid pbtxt config: {}", protoPath2);
    }

    dni::Graph* g = new dni::Graph(gc.value());

    std::string attack_result = "attack_res";
    spdlog::debug("Create ObserveOutputStream: {}", attack_result);
    g->ObserveOutputStream(attack_result);
    g->PrepareForRun();
    inject_after1(g, 0, 1, 0, pcapNamePath);
    g->RunOnce();
    g->Wait();
    auto attack_number = g->GetResult<double_t>(attack_result);
    spdlog::info("G2out {} result is: {}", attack_result, attack_number);
    g->Finish();

    return attack_number;
}

void inject_after2(
    dni::Graph* g, int after, int n, int interval, std::string pcapNamePath,
    std::vector<uint32_t> all_known_ips, std::vector<double> netdevs, std::string host_nic_name)
{
        std::this_thread::sleep_for(std::chrono::milliseconds(after));
        // std::vector<uint32_t> all_known_ips = {};

        for (int i = 0; i < n; i++)
        {
                // abnormal_nic_pcap_parser
                g->AddDatumToInputStream(
                    "pcapPath", dni::Datum(std::string(pcapNamePath)));

                g->AddDatumToInputStream("all_known_ips", dni::Datum(all_known_ips));

                g->AddDatumToInputStream(
                    "host_nic_name", dni::Datum(std::string(host_nic_name)));

                g->AddDatumToInputStream("netdevs_1", dni::Datum(netdevs));

                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        }
}



int main() {

    std::string hostName = "ddos21";
    std::string nicName = "enp1s0f1";
    std::string tableName = "packet";
    std::string tableName1 = "netdev";
    std::string tableName2 = "resource";
    const std::string& protoPath1 = "samples/ddos/realtime/pbtxt/graph1.pbtxt";
    const std::string& protoPath2 = "samples/ddos/realtime/pbtxt/graph2.pbtxt";
    const std::string& protoPath3 = "samples/ddos/realtime/pbtxt/graph3.pbtxt";
    std::string pcapDir = "/Users/guodong/work/nfs-data2/snding/splitData/";

    std::vector<uint32_t> all_known_ips = {};
    std::string host_nic_name = hostName + "#" + nicName;

    anomalyResult anomaly_result = anomaly_detect(hostName, nicName, tableName,
                tableName1, tableName2, protoPath1);

    if (anomaly_result.abnormal_number > 0) {
        std::string pcapNamePath = pcapDir + hostName + "/" + anomaly_result.pcapName;
        int attack_number = attack_detect(protoPath2, pcapNamePath);
        if (attack_number == 2) {
            spdlog::info("Confirmed Attack.");

            auto gc = dni::LoadTextprotoFile(protoPath3);
            if (!gc)
            {
                    spdlog::error("invalid pbtxt config: {}", protoPath3);
            }
            dni::Graph* g = new dni::Graph(gc.value());
            std::string out = "dms_rules";
            g->ObserveOutputStream(out);
            g->PrepareForRun();
            std::vector<double_t> netdevs;
            netdevs.push_back(anomaly_result.incrbytesRecv);
            netdevs.push_back(anomaly_result.inMbps_score);
            netdevs.push_back(anomaly_result.incrpacketRecv);
            netdevs.push_back(anomaly_result.inKpps_score);
            inject_after2(g, 0, 1, 0, pcapNamePath, all_known_ips, netdevs, host_nic_name);
            g->RunOnce();
            g->Wait();

            auto ret = g->GetResult<
            std::unordered_map<std::string, std::vector<dni::snding::DMSRule>>>(out);

            spdlog::info("G3out {} result is: {}", out, ret.size());
            for (auto&& pair : ret)
            {
                    spdlog::info(
                        "host-nicname:{}\t , dms-rule size: {}\n", pair.first,
                        pair.second.size());
                    for (auto&& rule : pair.second)
                    {
                            spdlog::info("{}\n", rule);
                    }
                    // spdlog::info("-----------------------------------\n");
            }

            g->Finish();

        } else if (attack_number == 1) {
            spdlog::info("Possible Attack.");
        } else {
            spdlog::info("No Attack.");
        }
    } else {
        spdlog::info("No Anomaly.");
    }

    spdlog::info("====END==== ");
    return 0;
}

