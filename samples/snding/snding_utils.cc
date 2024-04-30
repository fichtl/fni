#include "snding_utils.h"

#include <list>

// 根据题意b的匹配集为a的匹配集的子集等价于
//  a的前缀长度不大于b的且a和b的ip地址的前a.len位完全相同
bool isChildCollection(dni::IP& a, dni::IP& b)
{
        if (a.len > b.len)
                return false;
        if ((a.ip ^ b.ip) >> (32 - a.len))
                return false;
        return true;
}

void merge1(std::list<dni::IP>& ipl)
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

// 根据题意a与b为a`可以合并等价于a与b前缀长度相同且二者的前len位中只有最后一位不相同
bool unionCollection(dni::IP& a, dni::IP& b)
{
        if (a.len != b.len)
                return false;
        return ((a.ip ^ b.ip) >> (32 - a.len)) == 1u;
}

void merge2(std::list<dni::IP>& ipl)
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

std::vector<dni::IP> cidr_merge(const std::unordered_map<uint32_t, int>& ip_map)
{
        std::list<dni::IP> ipl;
        for (auto&& ip : ip_map)
        {
                ipl.emplace_back(ip.first, 32);
        }

        // for (auto it = ipl.begin(); it != ipl.end(); it++)
        // {
        //         printf("%x/%d\n", it->ip, it->len);
        // }

        ipl.sort([](const dni::IP& a, const dni::IP& b) {
                if (a.ip != b.ip)
                        return a.ip < b.ip;
                return a.len < b.len;
        });
        merge1(ipl);
        merge2(ipl);

        // std::vector<std::pair<uint32_t, int>> ret;
        // for (auto &&ip : ipl)
        // {
        //         ret.emplace_back(std::move(std::make_pair(ip.ip, ip.len)));
        // }

        std::vector<dni::IP> ret;
        for (auto&& ip : ipl)
        {
                ret.emplace_back(std::move(ip));
        }

        return std::move(ret);
}
