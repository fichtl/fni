#pragma once

#include <unordered_map>
#include <vector>

#include "snding_defines.h"

std::vector<dni::IP> cidr_merge(const std::unordered_map<uint32_t, int>& ip_map);
