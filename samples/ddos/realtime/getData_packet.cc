#include "influxdb.hpp"
#include <iostream>
#include <string>
#include <json/json.h>


// 定义一个结构体来封装返回值
struct PacketResult {
    double values[9]; // 用于存储9个double类型的数值
    std::string strValue; // 用于存储1个string类型的数值
};

// 函数定义
PacketResult getPacketStat(const std::string& tableName, const std::string& hostName, const std::string& nicName) {
     try {
        // 连接 InfluxDB 数据库
        influxdb_cpp::server_info insense("172.17.17.27", 8086, "snding", "", "");
        std::string resp_packet;
        std::string packet_sql = "select * from " + tableName + " where time >= now() - 3s and host = '" + hostName + "' and netif = '" + nicName + "' order by time desc limit 1";
        std::cout << packet_sql << std::endl;
        int success = influxdb_cpp::query(resp_packet, packet_sql, insense);
        std::cout << success << std::endl;
        if (success == 0) {
            // std::cout << "Query successful!" << std::endl;
            std::cout << resp_packet << std::endl;
        } else {
            std::cout << "Query failed." << std::endl;
        };

        // 创建 JSON 解析器
        Json::CharReaderBuilder readerBuilder;
        Json::Value root;
        std::string errs;
        // 解析 JSON 字符串
        std::istringstream s(resp_packet);
        if (!Json::parseFromStream(readerBuilder, s, &root, &errs)) {
            std::cerr << "Failed to parse JSON: " << errs << std::endl;
        }
        const Json::Value& results = root["results"];
        const Json::Value& series = results[0]["series"];
        const Json::Value& values = series[0]["values"];
        const Json::Value& firstValue = values[0];

        // Extract the desired values based on their indices in the 'columns' array
        std::string countTotalStr = std::to_string(firstValue[2].asInt());
        std::string countType_dstIPStr = std::to_string(firstValue[3].asInt());
        std::string countType_dstPortTcpStr = std::to_string(firstValue[4].asInt());
        std::string countType_dstPortUdpStr = std::to_string(firstValue[5].asInt());
        std::string countType_lengthStr = std::to_string(firstValue[6].asInt());
        std::string countType_protocolStr = std::to_string(firstValue[7].asInt());
        std::string countType_srcIPStr = std::to_string(firstValue[8].asInt());
        std::string countType_srcPortTcpStr = std::to_string(firstValue[9].asInt());
        std::string countType_srcPortUdpStr = std::to_string(firstValue[10].asInt());
        std::string pcapNameStr = firstValue[16].asString();

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

        // 初始化Result结构体
        PacketResult packet_stat;
        packet_stat.values[0] = countTotal;
        packet_stat.values[1] = countType_dstIP;
        packet_stat.values[2] = countType_dstPortTcp;
        packet_stat.values[3] = countType_dstPortUdp;
        packet_stat.values[4] = countType_length;
        packet_stat.values[5] = countType_protocol;
        packet_stat.values[6] = countType_srcIP;
        packet_stat.values[7] = countType_srcPortTcp;
        packet_stat.values[8] = countType_srcPortUdp;
        packet_stat.strValue = pcapNameStr;

        // 返回结果
        return packet_stat;

    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
    }

}


int main() {
    std::string tableName = "packet";
    std::string hostName = "ddos21";
    std::string nicName = "enp1s0f1";

    // 调用函数并获取结果
    PacketResult packet_stat = getPacketStat(tableName, hostName, nicName);

    // 输出结果
    for (int i = 0; i < 9; ++i) {
        std::cout << "Double value " << i + 1 << ": " << packet_stat.values[i] << std::endl;
    }
    std::cout << "String value: " << packet_stat.strValue << std::endl;

    return 0;
}

