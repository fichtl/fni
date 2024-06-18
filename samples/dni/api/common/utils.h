using namespace std;

#include <string>

namespace dni {
        class Utils {
        public:
                static std::string uuid();

                static long now();

                static std::string getServerHost();
                static std::string getBackend();

                static std::string getServerPort();
                static std::string getBackendPort();

                static std::string getSecure();
        };
}   // namespace dni
