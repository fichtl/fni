#include <iostream>

#include "dni/framework/framework.h"
#include "google/protobuf/text_format.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"

int main()
{
        std::string path = "samples/dni/serdes/testdata/graph1.pbtxt";

        auto gc = dni::LoadTextprotoFile(path);
        if (!gc.has_value())
        {
                std::cout << "invalid pbtxt format, pbtxt path: " << path << std::endl;
                return -1;
        }

        google::protobuf::io::OstreamOutputStream o(&std::cout, -1);
        google::protobuf::TextFormat::Print(gc.value(), &o);

        std::cout << "parse success!" << std::endl;

        return 0;
}
