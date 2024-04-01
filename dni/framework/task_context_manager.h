#pragma once

#include <memory>

#include "dni/framework/task_context.h"
#include "dni/framework/task_state.h"
#include "dni/framework/utils/tags.h"
#include "fmt/format.h"

namespace dni {

        class TaskContextManager {
        public:
                void Initialize(
                    TaskState* state, std::shared_ptr<utils::TagMap> input_tag_map,
                    std::shared_ptr<utils::TagMap> output_tag_map, bool in_parallel);

                // Prepare default TaskContext before graph start processing.
                int PrepareForRun();

                void Finish();

                TaskContext* DefaultContext() const { return default_ctx_.get(); }

        private:
                TaskState* state_;
                std::shared_ptr<utils::TagMap> input_tag_map_;
                std::shared_ptr<utils::TagMap> output_tag_map_;
                bool in_parallel_;

                std::unique_ptr<TaskContext> default_ctx_;

                friend class fmt::formatter<dni::TaskContextManager>;
        };

}   // namespace dni

namespace fmt {

        template <>
        struct formatter<dni::TaskContextManager>: formatter<std::string_view> {
                auto format(
                    const dni::TaskContextManager& mngr, format_context& ctx) const
                {
                        format_to(ctx.out(), "state({})", *mngr.state_);
                        if (mngr.default_ctx_)
                        {
                                format_to(
                                    ctx.out(), ",default_ctx({})", mngr.default_ctx_);
                        }
                        return ctx.out();
                }
        };

}   // namespace fmt
