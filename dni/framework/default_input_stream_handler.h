#pragma once

#include <memory>
#include <vector>

#include "dni/framework/input_stream_handler.h"

namespace dni {

        class DefaultInputStreamHandler: public InputStreamHandler {
        public:
                DefaultInputStreamHandler() = delete;

                DefaultInputStreamHandler(int managers_count);

                DefaultInputStreamHandler(
                    std::shared_ptr<utils::TagMap> tag_map,
                    ContextManager* context_manager, bool in_parallel);

                DataReadiness GetReadiness(std::time_t* ts) override;

                void Marshal(InputStreamSet* inputs) override;

        protected:
                SyncSet sync_set_;
        };

}   // namespace dni
