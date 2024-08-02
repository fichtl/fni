#include "cidr.h"

#include <list>

#include "spdlog/spdlog.h"

namespace dni {

        // cidr merge function 1, merge vector of ips

        // b的匹配集为a的匹配集的子集等价于
        // a的前缀长度不大于b的且a和b的ip地址的前a.len位完全相同
        bool isChildCollection(CIDR& a, CIDR& b)
        {
                if (a.len > b.len)
                        return false;
                if ((a.ip ^ b.ip) >> (32 - a.len))
                        return false;
                return true;
        }

        // 根据题意a与b为a`可以合并等价于
        //  a与b前缀长度相同且二者的前len位中只有最后一位不相同
        bool unionCollection(CIDR& a, CIDR& b)
        {
                if (a.len != b.len)
                        return false;
                return ((a.ip ^ b.ip) >> (32 - a.len)) == 1u;
        }

        void first_step_merge(std::list<CIDR>& ipl)
        {
                auto i = ipl.begin(), j = ipl.begin();
                j++;
                while (j != ipl.end())
                {
                        if (isChildCollection(*i, *j))
                                j = ipl.erase(j);
                        else
                        {
                                i++;
                                j++;
                        }
                }
        }

        void second_step_merge(std::list<CIDR>& ipl)
        {
                auto i = ipl.begin(), j = ipl.begin();
                j++;
                while (j != ipl.end())
                {
                        if (unionCollection(*i, *j))
                        {
                                j = ipl.erase(j);
                                ((*i).len)--;
                                if (i != ipl.begin())
                                {
                                        i--;
                                        j--;
                                }
                        }
                        else
                        {
                                i++;
                                j++;
                        }
                }
        }

        std::vector<CIDR> cidr_merge(const std::unordered_map<uint32_t, int>& ip_map)
        {
                std::list<CIDR> ipl;
                for (auto&& ip : ip_map)
                {
                        ipl.emplace_back(ip.first, 32);
                }

                ipl.sort([](const CIDR& a, const CIDR& b) {
                        if (a.ip != b.ip)
                                return a.ip < b.ip;
                        return a.len < b.len;
                });
                first_step_merge(ipl);
                second_step_merge(ipl);

                std::vector<CIDR> ret;
                for (auto&& ip : ipl)
                {
                        ret.emplace_back(std::move(ip));
                }

                return std::move(ret);
        }

        // cidr merge function 2, merge ip range from A to B

        // Set a bit to a given value (0 or 1); MSB is bit 1, LSB is bit 32
        uint32_t set_bit(uint32_t ip, int bitno, int val)
        {
                if (val)
                        return (ip | (1 << (32 - bitno)));
                else
                        return (ip & ~(1 << (32 - bitno)));
        }

        // Compute netmask address given length
        uint32_t netmask(int len)
        {
                if (len == 0)
                        return (~((uint32_t) -1));
                else
                        return (~((1 << (32 - len)) - 1));
        }

        // Compute broadcast address given address and len
        uint32_t broadcast(uint32_t ip, int len) { return (ip | ~netmask(len)); }

        // Compute network address given address and len */
        uint32_t network(uint32_t ip, int len) { return (ip & netmask(len)); }

        // Recursively compute network addresses to cover range lo-hi
        /* Note: Worst case scenario is when lo=0.0.0.1 and hi=255.255.255.254
         *       We then have 62 CIDR bloks to cover this interval, and 125
         *       calls to split_range();
         *       The maximum possible recursion depth is 32.
         */
        int split_range(
            uint32_t ip, int len, uint32_t lo, uint32_t hi, std::vector<CIDR>& CIDRs)
        {
                uint32_t bc, lower_half, upper_half;

                if ((len < 0) || (len > 32))
                {
                        SPDLOG_ERROR("Invalid mask size {}!", len);
                        return -1;
                }

                bc = broadcast(ip, len);

                if ((lo < ip) || (hi > bc))
                {
                        SPDLOG_ERROR(
                            "Out of range limits: {:X}, {:X} for network {:X}/{}, "
                            "broadcast: "
                            "{:X}!",
                            lo, hi, ip, len, bc);
                        return -1;
                }

                if ((lo == ip) && (hi == bc))
                {
                        CIDRs.emplace_back(ip, len);
                        // SPDLOG_DEBUG("CIDRs size: {}", CIDRs.size());

                        return 0;
                }

                len++;
                lower_half = ip;
                upper_half = set_bit(ip, len, 1);

                if (hi < upper_half)
                {
                        if (split_range(lower_half, len, lo, hi, CIDRs) == -1)
                                return -1;
                }
                else if (lo >= upper_half)
                {
                        if (split_range(upper_half, len, lo, hi, CIDRs) == -1)
                                return -1;
                }
                else
                {
                        if (split_range(
                                lower_half, len, lo, broadcast(lower_half, len), CIDRs) ==
                            -1)
                                return -1;
                        if (split_range(upper_half, len, upper_half, hi, CIDRs) == -1)
                                return -1;
                }

                return 0;
        }

        std::vector<CIDR> iprange_to_cidrs(CIDR& from, CIDR& to)
        {
                std::vector<CIDR> CIDRs;

                if (from.len > 32 || to.len > 32)
                {
                        SPDLOG_ERROR(
                            "len bigger than 32, from.len: {}, to.len: {}", from.len,
                            to.len);
                        return std::move(CIDRs);
                }

                uint32_t lo, hi;

                lo = network(from.ip, from.len);
                hi = broadcast(to.ip, to.len);

                if (split_range(0, 0, lo, hi, CIDRs) == -1)
                {
                        CIDRs.clear();
                }

                // SPDLOG_DEBUG("CIDRs size: {}", CIDRs.size());

                return std::move(CIDRs);
        }

}   // namespace dni
