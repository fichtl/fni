#include "dni/framework/default_input_stream_handler.h"

namespace dni {

        std::vector<int> GetIds(const std::shared_ptr<utils::TagMap>& tag_map)
        {
                std::vector<int> result;

                return result;
        }

        DefaultInputStreamHandler::DefaultInputStreamHandler(int managers_count)
            : InputStreamHandler(managers_count), sync_set_(this, GetIds(GetTagMap()))
        {}

        DefaultInputStreamHandler::DefaultInputStreamHandler(
            std::shared_ptr<utils::TagMap> tag_map, ContextManager* context_manager,
            bool in_parallel)
            : InputStreamHandler(std::move(tag_map), context_manager, in_parallel),
              sync_set_(this, GetIds(GetTagMap()))
        {}

        DataReadiness DefaultInputStreamHandler::GetReadiness(std::time_t* ts)
        {
                return sync_set_.GetReadiness(ts);
        }

        void DefaultInputStreamHandler::Marshal(InputStreamSet* inputs)
        {
                sync_set_.Sync(inputs);
        }

}   // namespace dni
