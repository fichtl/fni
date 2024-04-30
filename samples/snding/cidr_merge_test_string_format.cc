#include <iostream>
#include <list>
#include <string>
#include <vector>

using namespace std;

//有穷状态机的状态，分别对应数字和符号
//由于ip地址起始位置必为数字，故可以直接设置NUM为起始状态
enum STATE { NUM, SIGN };

typedef struct IP {
        unsigned ip = 0;
        int len = 32;
} IP;

//读入并处理ip地址字符串
IP read(string s)
{
        IP a;
        bool flag = false;   //记录是否已经对斜线符号进行了处理
        STATE state = NUM;
        unsigned buf = 0;             //缓存每次数据处理的结果
        unsigned rule = 0x01000000;   //在存储数据到对应字节时进行位移所用的变量
        for (int i = 0; i < s.size(); i++)
        {
                switch (state)
                {
                case NUM:   //处理数字
                        if (isdigit(s[i]))
                                buf = s[i] - '0' + buf * 10;   //是数字就纳入缓存
                        else   //否则要令指针回退一格并转移状态到SIGN
                        {
                                i--;
                                state = SIGN;
                        }
                        break;
                case SIGN:   //处理符号
                        if (isdigit(s[i]))
                        {
                                i--;
                                state = NUM;
                                break;
                        }
                        a.ip += buf * rule;   //将缓存数据移动到正确位置并加入到ip中
                        rule >>= 8;   //下一个数据应位移的位数
                        buf = 0;      //缓存清空
                        flag = (s[i] == '/') ? true : false;
                        break;
                }
                if (i == s.size() - 1)   //字符串在遍历到尾部时需要进行特殊处理
                {
                        if (flag)   //根据题意，如果已经处理过斜线则说明此ip地址是标准型或后缀省略型
                        {
                                a.len = buf;   //此时buf存放的必定是前缀长度
                        }
                        else   //否则此ip地址为长度省略型，此时buf存放的仍是ip地址数据，前缀长度需要自己计算
                        {
                                a.ip += buf * rule;
                                rule >>= 8;
                                while (rule)
                                {
                                        a.len -= 8;
                                        rule >>= 8;
                                }
                        }
                }
        }
        return a;
}

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
        {
                std::vector<std::string> ip_vec;
                std::string start = "1.2.0.";
                for (size_t i = 0; i < 256; i++)
                {

                        std::string ip_d = to_string(i);
                        ip_vec.push_back(start + ip_d);
                }

                //

                list<IP> ipl;
                for (int i = 0; i < ip_vec.size(); i++)
                {
                        ipl.push_back(read(ip_vec[i]));
                }

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
        }

        cout << endl << endl << endl;

        {
                std::vector<std::string> ip_vec;
                std::string start = "1.2.0.";
                for (size_t i = 0; i < 256; i++)
                {

                        std::string ip_d = to_string(i);
                        ip_vec.push_back(start + ip_d);
                }
                //
                start = "1.2.1.";
                for (size_t i = 0; i < 256; i++)
                {

                        std::string ip_d = to_string(i);
                        ip_vec.push_back(start + ip_d);
                }
                //
                start = "1.2.2.";
                for (size_t i = 0; i < 256; i++)
                {

                        std::string ip_d = to_string(i);
                        ip_vec.push_back(start + ip_d);
                }
                //
                start = "1.2.3.";
                for (size_t i = 0; i < 256; i++)
                {

                        std::string ip_d = to_string(i);
                        ip_vec.push_back(start + ip_d);
                }
                //
                start = "1.2.4.";
                for (size_t i = 0; i < 256; i++)
                {

                        std::string ip_d = to_string(i);
                        ip_vec.push_back(start + ip_d);
                }
                //
                start = "1.2.5.";
                for (size_t i = 0; i < 256; i++)
                {

                        std::string ip_d = to_string(i);
                        ip_vec.push_back(start + ip_d);
                }
                //
                start = "1.2.6.";
                for (size_t i = 0; i < 256; i++)
                {

                        std::string ip_d = to_string(i);
                        ip_vec.push_back(start + ip_d);
                }
                //
                start = "1.2.7.";
                for (size_t i = 0; i < 256; i++)
                {

                        std::string ip_d = to_string(i);
                        ip_vec.push_back(start + ip_d);
                }

                //

                list<IP> ipl;
                for (int i = 0; i < ip_vec.size(); i++)
                {
                        ipl.push_back(read(ip_vec[i]));
                }

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
        }



        return 0;
}
