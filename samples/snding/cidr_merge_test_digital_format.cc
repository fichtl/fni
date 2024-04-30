#include <iostream>
#include <list>
#include <string>
#include <vector>

using namespace std;

//有穷状态机的状态，分别对应数字和符号
//由于ip地址起始位置必为数字，故可以直接设置NUM为起始状态
enum STATE { NUM, SIGN };

struct IP {
        IP() = delete;
        IP(uint32_t ip, int len): ip(ip), len(len) {}

        uint32_t ip;
        int len;
};

//根据题意b的匹配集为a的匹配集的子集等价于
// a的前缀长度不大于b的且a和b的ip地址的前a.len位完全相同
bool isChildCollection(IP& a, IP& b)
{
        if (a.len > b.len)
                return false;
        if ((a.ip ^ b.ip) >> (32 - a.len))
                return false;
        return true;
}

void merge1(list<IP>& ipl)
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

//根据题意a与b为a`可以合并等价于
// a与b前缀长度相同且二者的前len位中只有最后一位不相同
bool unionCollection(IP& a, IP& b)
{
        if (a.len != b.len)
                return false;
        return ((a.ip ^ b.ip) >> (32 - a.len)) == 1u;
}

void merge2(list<IP>& ipl)
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


int main()
{
        std::unordered_map<uint32_t, int> ip_map;
        uint32_t start = 0x01020000;
        for (size_t i = 0; i < 256; i++)
        {
                ip_map[start + i] = 10;
        }

        //
        start = 0x01020100;
        for (size_t i = 0; i < 256; i++)
        {
                ip_map[start + i] = 10;
        }

        //
        start = 0x01020200;
        for (size_t i = 0; i < 256; i++)
        {
                ip_map[start + i] = 10;
        }

        //
        start = 0x01020300;
        for (size_t i = 0; i < 256; i++)
        {
                ip_map[start + i] = 10;
        }

        //
        start = 0x01020400;
        for (size_t i = 0; i < 256; i++)
        {
                ip_map[start + i] = 10;
        }

        //
        start = 0x01020500;
        for (size_t i = 0; i < 256; i++)
        {
                ip_map[start + i] = 10;
        }

        //
        start = 0x01020600;
        for (size_t i = 0; i < 256; i++)
        {
                ip_map[start + i] = 10;
        }

        //
        start = 0x01020700;
        for (size_t i = 0; i < 256; i++)
        {
                ip_map[start + i] = 10;
        }

        list<IP> ipl;
        for (auto &&ip : ip_map)
        {
                ipl.emplace_back(ip.first, 32);
        }
        
        // for (auto it = ipl.begin(); it != ipl.end(); it++)
        // {
        //         printf("%x/%d\n", it->ip, it->len);
        // }

        ipl.sort([](const IP& a, const IP& b) {
                if (a.ip != b.ip)
                        return a.ip < b.ip;
                return a.len < b.len;
        });
        merge1(ipl);
        merge2(ipl);


        for (auto& e : ipl)
        {
                unsigned a = 0xff000000, b = 0x00ff0000, c = 0x0000ff00, d = 0x000000ff;
                cout << ((e.ip & a) >> 24) << '.' << ((e.ip & b) >> 16) << '.'
                     << ((e.ip & c) >> 8) << '.' << ((e.ip & d)) << '/' << e.len << endl;
        }
        return 0;
}
