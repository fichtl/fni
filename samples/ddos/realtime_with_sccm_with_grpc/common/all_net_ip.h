#pragma once

#include <string>
#include <vector>

namespace dni {
void get_all_net_ips(
    std::vector<unsigned int>& all_known_ips, std::vector<std::string>& host_nic_names);
}