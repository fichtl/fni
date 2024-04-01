#include "dni/framework/output_stream_manager.h"

#include <list>
#include <mutex>
#include <string>
#include <vector>

#include "dni/framework/datum.h"
#include "dni/framework/dtype.h"
#include "dni/framework/input_stream_handler.h"
#include "dni/framework/output_stream.h"

namespace dni {

        int OutputStreamManager::Initialize(const std::string& name, const Dtype* type)
        {
                output_stream_spec_.name = name;
                output_stream_spec_.datum_type = type;
                return 0;
        }

        // TODO: not fully implemented.
        void OutputStreamManager::PrepareForRun() {}

        void OutputStreamManager::AddMirror(InputStreamHandler* ish, int id)
        {
                mirrors_.emplace_back(ish, id);
        }

        // TODO: not fully implemented.
        void OutputStreamManager::Close() {}

}   // namespace dni
