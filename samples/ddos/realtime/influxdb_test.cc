#include "influxdb.hpp"
#include <iostream>
#include <string>


int main()
{
    try {
        std::cout << "===A===" << std::endl;
        // 创建InfluxDB客户端实例
        influxdb_cpp::server_info si("172.17.17.27", 8086, "snding", "", "");
        std::cout << "===B===" << std::endl;

        // 执行查询
        std::string resp;
        int success = influxdb_cpp::query(resp, "select * from packet "
                            "where time >= now() - 3s and ip = '10.17.1.21' "
                            "order by time desc limit 1", si);
        // std::cout << success << std::endl;
        if (success == 0) {
            // std::cout << "Query successful!" << std::endl;
            std::cout << resp << std::endl;
        } else {
            std::cout << "Query failed." << std::endl;
        };

        // std::cout << resp << std::endl;

        // 检查结果并打印
        std::cout << "===C===" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
    }

     return 0;

}
