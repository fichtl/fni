#include "influxdb.hpp"
#include <iostream>
#include <string>
#include <json/json.h>



int main() {
    try {
        std::cout << "===A===" << std::endl;
        // 创建InfluxDB客户端实例
        influxdb_cpp::server_info insense("172.17.17.27", 8086, "snding", "", "");
        std::cout << "===B===" << std::endl;

        // 执行查询
        std::string resp;
        int success = influxdb_cpp::query(resp, "select * from packet "
                                    "where time >= now() - 3s and "
                                    "host = 'ddos21' and "
                                    "netif = 'enp1s0f1' "
                                    "order by time desc limit 1", insense);
        if (success == 0) {
            // std::cout << "Query successful!" << std::endl;
            std::cout << resp << std::endl;
        } else {
            std::cout << "Query failed." << std::endl;
        };

        // 创建 JSON 解析器
        Json::CharReaderBuilder readerBuilder;
        Json::Value root;
        std::string errs;

        // 解析 JSON 字符串
        std::istringstream s(resp);
        if (!Json::parseFromStream(readerBuilder, s, &root, &errs)) {
            std::cerr << "Failed to parse JSON: " << errs << std::endl;
            return 1;
        }

        // 获取 countTotal 的值
        try {
            const Json::Value& results = root["results"];
            const Json::Value& series = results[0]["series"];
            const Json::Value& values = series[0]["values"];
            const Json::Value& firstValueArray = values[0];

            // 获取 "countTotal" 在 "columns" 中的索引
            const Json::Value& columns = series[0]["columns"];
            int countTotalIndex = -1;
            for (Json::ArrayIndex i = 0; i < columns.size(); ++i) {
                if (columns[i].asString() == "countTotal") {
                    countTotalIndex = i;
                    break;
                }
            }

            if (countTotalIndex == -1) {
                std::cerr << "countTotal not found in columns" << std::endl;
                return 1;
            }

            // 获取 countTotal 的值
            int countTotal1 = firstValueArray[countTotalIndex].asInt();
            double countTotal = static_cast<double>(countTotal1);
            std::cout << "countTotal: " << countTotal << std::endl;

        } catch (const std::exception& e) {
            std::cerr << "Exception: " << e.what() << std::endl;
            return 1;
        }


        // 检查结果并打印
        std::cout << "===C===" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}

