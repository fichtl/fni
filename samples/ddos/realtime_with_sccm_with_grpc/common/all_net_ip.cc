#include "all_net_ip.h"

#include <iostream>
#include <json/json.h>
#include <regex>
#include <string>
#include <unordered_map>
#include <vector>

#include "InfluxDB.h"
#include "InfluxDBFactory.h"
#include "spdlog/spdlog.h"

namespace dni {

        std::regex ipv4_pattern("((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]"
                                "|2[0-4][0-9]|[01]?[0-9][0-9]?)");

        void split_string(
            const std::string& str, const std::string& split,
            std::vector<std::string>& res)
        {
                // std::regex ws_re("\\s+"); // 正则表达式,匹配空格
                std::regex reg(split);   // 匹配split
                std::sregex_token_iterator pos(str.begin(), str.end(), reg, -1);
                decltype(pos) end;   // 自动推导类型
                for (; pos != end; ++pos)
                {
                        res.push_back(pos->str());
                }
        }

        void get_all_net_ips(
            std::vector<unsigned int>& all_known_ips,
            std::vector<std::string>& host_nic_names)
        {
                auto client = influxdb::InfluxDBFactory::Get(
                    "http://172.17.17.33:8086?db=sccmmaster");

                // auto query_ret = client->execute("select netif from baseinfo");
                // auto query_ret = client->execute("select netif from baseinfo Group by "
                //                                  "host_id order by time desc limit 1");

                auto query_ret =
                    client->execute("select netif,host_name from baseinfo Group by "
                                    "host_id order by time desc limit 1");

                // std::cout << query_ret << std::endl;

                Json::CharReaderBuilder readerBuilder;
                Json::Value root;
                std::string errs;
                std::istringstream s(query_ret);
                if (!Json::parseFromStream(readerBuilder, s, &root, &errs))
                {
                        std::cerr << "Failed to parse JSON: " << errs << std::endl;
                }

                const Json::Value& series = root["results"][0]["series"];
                // std::cout << series.size() << std::endl;

                for (int i = 0; i < series.size(); i++)
                {
                        const Json::Value& json_host_name = series[i]["values"][0][2];
                        auto host_name = json_host_name.asString();

                        const Json::Value& ret = series[i]["values"][0][1];

                        std::string m = ret.asString();

                        std::string ips_str = m.substr(4, m.size() - 5);

                        std::vector<std::string> ips_in_one_dev;
                        split_string(ips_str, " ", ips_in_one_dev);

                        for (auto&& if_ip : ips_in_one_dev)
                        {
                                auto pos = if_ip.find_first_of(":");
                                auto nic_name = if_ip.substr(0, pos);
                                auto ip = if_ip.substr(pos + 1);

                                if (std::regex_match(ip, ipv4_pattern))
                                {
                                        // std::cout << ip << std::endl;

                                        std::vector<std::string> ip_segs;
                                        split_string(ip, "\\.", ip_segs);
                                        unsigned int ip_num =
                                            (std::stoul(ip_segs[0]) << 24) +
                                            (std::stoul(ip_segs[1]) << 16) +
                                            (std::stoul(ip_segs[2]) << 8) +
                                            std::stoul(ip_segs[3]);
                                        // spdlog::info(
                                        //     "{:x}, {}", ip_num,
                                        //     host_name + "#" + nic_name);

                                        all_known_ips.push_back(ip_num);
                                        host_nic_names.push_back(
                                            host_name + "#" + nic_name);
                                }
                        }

                        // spdlog::info("\n\n");
                }
        }
}   // namespace dni