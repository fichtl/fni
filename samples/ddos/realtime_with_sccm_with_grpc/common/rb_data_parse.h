#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace dni {

        struct RBDataHeader {
                RBDataHeader() = default;

                std::string host_nic_name;
                uint64_t ts;

                std::vector<uint32_t> pkts_stats;

                std::vector<double> netdev_stats;
                int64_t speed;

                double cur_cpu;
                std::vector<int64_t> resource_stats;

                uint32_t nic_ip;
                uint32_t mgr_ip;

                std::vector<int64_t> additional_stats;
        };

        void parse_header(unsigned char* rbdata, RBDataHeader& rb_header, int& offset);

        void parse_packets(
            unsigned char* pktsdata, uint32_t cnt,
            std::vector<std::vector<uint32_t>>& packets);

}   // namespace dni
