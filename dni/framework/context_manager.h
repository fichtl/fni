#pragma once

#include <memory>
#include <mutex>

#include "dni/framework/context.h"
#include "dni/framework/task_state.h"
#include "dni/framework/utils/tags.h"

namespace dni {

        class ContextManager {
        public:
                void Initialize(
                    TaskState* state, std::shared_ptr<utils::TagMap> input_tag_map,
                    std::shared_ptr<utils::TagMap> output_tag_map, bool in_parallel);

        private:
                TaskState* state_;
                std::shared_ptr<utils::TagMap> input_tag_map_;
                std::shared_ptr<utils::TagMap> output_tag_map_;
                bool in_parallel_;

                std::unique_ptr<Context> default_ctx_;
        };

}   // namespace dni
