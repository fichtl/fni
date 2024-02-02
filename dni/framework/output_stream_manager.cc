#include "dni/framework/output_stream_manager.h"

#include <list>
#include <mutex>
#include <string>
#include <vector>

#include "dni/framework/dtype.h"
#include "dni/framework/datum.h"
#include "dni/framework/input_stream_handler.h"
#include "dni/framework/output_stream.h"

namespace dni {

        // class OutputStreamManager {
        // public:
        //         int Initialize(const std::string& name, const DataType* type);

        //         void PrepareForRun();

        //         void AddMirror(InputStreamHandler* input_stream_handler, int id);

        //         void Close();

        //         OutputStreamSpec* Spec() { return &output_stream_spec_; }

        // private:
        //         struct Mirror {
        //                 Mirror(InputStreamHandler* input_stream_handler, int id)
        //                     : input_stream_handler(input_stream_handler), id(id)
        //                 {}
        //                 InputStreamHandler* input_stream_handler;
        //                 int id;
        //         };

        //         std::string name_;
        //         std::mutex mu_;

        //         std::vector<Mirror> mirrors_;

        //         OutputStreamSpec output_stream_spec_;
        // };

}   // namespace dni
