using namespace std;

#include "samples/grpc/protos/landing.grpc.pb.h"

using dni::samples::grpc::TalkRequest;

#include <list>

namespace dni {
        class Utils {
        public:
                static std::string uuid();

                static std::list<TalkRequest> buildLinkRequests(
                    std::vector<std::string> pbtxts);

                static std::list<TalkRequest> buildLinkDataRequests(
                    std::vector<double_t> data);

                static long now();

                static std::string getServerHost();
                static std::string getBackend();

                static std::string getServerPort();
                static std::string getBackendPort();

                static std::string getSecure();
        };
}   // namespace dni
