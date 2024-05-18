#pragma once

#include <unordered_map>
#include <vector>

#include "fmt/format.h"

namespace dni {

struct CIDR {
        CIDR() = default;
        CIDR(uint32_t ip, int len): ip(ip), len(len) {}

        uint32_t ip = 0;
        int len = 32;

        bool operator==(const CIDR& rhl) const
        {
                if ((ip == rhl.ip) && (len == rhl.len))
                        return true;
                else
                        return false;
        }
};

std::vector<CIDR> cidr_merge(const std::unordered_map<uint32_t, int>& ip_map);

std::vector<CIDR> iprange_to_cidrs(CIDR from, CIDR to);

}   // namespace dni

namespace fmt {

template <>
struct formatter<dni::CIDR>: formatter<std::string_view> {
        auto format(const dni::CIDR& ip, format_context& ctx) const
        {
                return format_to(ctx.out(), "cidr-ip {:X}/{}", ip.ip, ip.len);
        }
};

}   // namespace fmt
