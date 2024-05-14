#include "dni/framework/framework.h"

int main()
{
        std::string path = "samples/dni/serdes/testdata/graph1.pbtxt";

        auto gc = dni::LoadTextprotoFile(path);
        if (!gc.has_value())
        {
                std::cout << "invalid pbtxt format, pbtxt path: " << path << std::endl;
                return -1;
        }

        dni::Graph* g = new dni::Graph(gc.value());

        std::cout << "parse success!" << std::endl;

        return 0;
}
