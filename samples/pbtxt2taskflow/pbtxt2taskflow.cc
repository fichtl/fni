#include "dni/framework/framework.h"

// for read_from_pbtxt_nocopy
// #include <fcntl.h>
// #include <google/protobuf/io/zero_copy_stream_impl.h>

int main()
{
        std::string path = "samples/pbtxt2taskflow/testdata/pbtxt2taskflow.pbtxt";

        auto gc = dni::ParsePbtxtToGraphConfig(path);
        if (!gc.has_value())
        {
                std::cout << "invalid pbtxt format, pbtxt path: " << path << std::endl;
                return -1;
        }

        dni::Graph* g = new dni::Graph(gc.value());

        std::cout << "parse success!" << std::endl;

        return 0;
}
