#include <fstream>
#include <iostream>
#include <json/json.h>
#include <string>

int main()
{
        std::ifstream ifs;
        ifs.open("samples/snding/testdata/ceni-40nodes-topology.json");
        if (!ifs.is_open())
        {
                std::cout << "ifs.open failed" << "\n";
                return -1;
        }

        Json::Reader reader;
        Json::Value root;
        // 解析到root，root将包含Json里所有子元素
        if (!reader.parse(ifs, root, false))
        {
                std::cout << "parse failed" << "\n";
                return -1;
        }
        // 实际字段保存在这里，因为知道是什么类型，所以直接用asString()，没有进行类型的判断
        std::string network_id = root["network_id"].asString();
        std::string network_name = root["network_name"].asString();
        std::cout << network_id << ": " << network_name << "\n";
        ifs.close();

        return 0;
}

#if 0

int main()
{
        std::ifstream ifs;
        ifs.open("samples/snding/testdata/checkjson.json");
        if (!ifs.is_open())
        {
                std::cout << "ifs.open failed" << "\n";
                return -1;
        }

        Json::Reader reader;
        Json::Value root;
        // 解析到root，root将包含Json里所有子元素
        if (!reader.parse(ifs, root, false))
        {
                std::cout << "parse failed" << "\n";
                return -1;
        }
        // 实际字段保存在这里，因为知道是什么类型，所以直接用asString()，没有进行类型的判断
        std::string name = root["name"].asString();
        int age = root["age"].asInt();   // 这是整型，转化是指定类型
        std::cout << name << ":" << age << "\n";
        ifs.close();

        return 0;
}

#endif
