#pragma once

#include <memory>

#include "dni/framework/context_manager.h"
#include "dni/framework/output_stream.h"
#include "dni/framework/output_stream_manager.h"
#include "dni/framework/utils/tags.h"

namespace dni {

        class OutputStreamHandler {
        public:
                OutputStreamHandler(
                    std::shared_ptr<utils::TagMap> tag_map,
                    ContextManager* context_manager, bool in_parallel);

                void PrepareForRun();

                void Open(OutputStreamSet* outputs);

                void Close(OutputStreamSet* outputs);

                virtual void Propagate();

                int NStreams() { return input_stream_managers_.size(); }

        protected:
                OutputStreamManagerSet output_stream_managers_;
                ContextManager* const context_manager_;
                const bool in_parallel_;
        };

}   // namespace dni
