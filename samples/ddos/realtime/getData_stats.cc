#include "influxdb.hpp"
#include <iostream>
#include <string>
#include <json/json.h>


// get nic(packet/netdev) stat data
Json::Value getNicStat(const std::string& tableName, const std::string& hostName, const std::string& nicName) {
     try {
        // 连接 InfluxDB 数据库
        influxdb_cpp::server_info insense("172.17.17.27", 8086, "snding", "", "");
        std::string resp_nic;
        std::string nic_sql = "select * from " + tableName + " where time >= now() - 3s and host = '" + hostName + "' and netif = '" + nicName + "' order by time desc limit 1";
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
Json::Value getHostStat(const std::string& tableName, const std::string& hostName) {
     try {
        // 连接 InfluxDB 数据库
        influxdb_cpp::server_info insense("172.17.17.27", 8086, "snding", "", "");
        std::string resp_host;
        std::string host_sql = "select * from " + tableName + " where time >= now() - 3s and host = '" + hostName +  "' order by time desc limit 1";
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

void inject_after()
{

    std::string hostName = "ddos21";
    std::string nicName = "enp1s0f1";
    std::string tableName = "packet";
    std::string tableName1 = "netdev";
    std::string tableName2 = "resource";

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

}



int main() {

    // std::string hostName = "ddos21";
    // std::string nicName = "enp1s0f1";
    // std::string tableName = "packet";
    // std::string tableName1 = "netdev";
    // std::string tableName2 = "resource";
    // Json::Value packetStatValue = getNicStat(tableName, hostName, nicName);
    // std::cout << packetStatValue.toStyledString() << std::endl;
    // Json::Value netdevStatValue = getNicStat(tableName1, hostName, nicName);
    // std::cout << netdevStatValue.toStyledString() << std::endl;
    // Json::Value resourceStatValue = getHostStat(tableName2, hostName);
    // std::cout << resourceStatValue.toStyledString() << std::endl;

    inject_after();

    return 0;
}

