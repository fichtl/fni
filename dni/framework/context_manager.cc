#include "dni/framework/context_manager.h"

#include <memory>

#include "dni/framework/task_state.h"
#include "dni/framework/utils/tags.h"

namespace dni {

        void ContextManager::Initialize(
            TaskState* state, std::shared_ptr<utils::TagMap> input_tag_map,
            std::shared_ptr<utils::TagMap> output_tag_map, bool in_parallel)
        {
                state_ = state;
                input_tag_map_ = std::move(input_tag_map);
                output_tag_map_ = std::move(output_tag_map);
                in_parallel_ = in_parallel;
        }

        int ContextManager::PrepareForRun()
        {
                default_ctx_ =
                    std::make_unique<Context>(state_, input_tag_map_, output_tag_map_);
                return 0;
        }

        void ContextManager::Finish() { default_ctx_ = nullptr; }

}   // namespace dni
