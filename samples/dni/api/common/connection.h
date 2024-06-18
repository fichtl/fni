#include <grpcpp/grpcpp.h>

using namespace std;
using grpc::Channel;

namespace dni {
        class Connection {
        public:
                static std::string getFileContent(const char* path);

                static shared_ptr<Channel> getChannel();
        };
}   // namespace dni
