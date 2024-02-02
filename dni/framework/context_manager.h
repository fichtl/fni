#pragma once

#include <memory>

#include "dni/framework/context.h"
#include "dni/framework/task_state.h"
#include "dni/framework/utils/tags.h"
#include "fmt/format.h"

namespace dni {

        class ContextManager {
        public:
                void Initialize(
                    TaskState* state, std::shared_ptr<utils::TagMap> input_tag_map,
                    std::shared_ptr<utils::TagMap> output_tag_map, bool in_parallel);

                int PrepareForRun();

                void Finish();

                Context* DefaultContext() const { return default_ctx_.get(); }

        private:
                TaskState* state_;
                std::shared_ptr<utils::TagMap> input_tag_map_;
                std::shared_ptr<utils::TagMap> output_tag_map_;
                bool in_parallel_;

                std::unique_ptr<Context> default_ctx_;

                friend class fmt::formatter<dni::ContextManager>;
        };

}   // namespace dni

namespace fmt {

        template <>
        struct formatter<dni::ContextManager>: formatter<std::string_view> {
                auto format(const dni::ContextManager& mngr, format_context& ctx) const
                {
                        format_to(ctx.out(), "state({:p})", fmt::ptr(mngr.state_));
                        if (mngr.default_ctx_)
                        {
                                format_to(
                                    ctx.out(), ",default_ctx({})", mngr.default_ctx_);
                        }
                        return ctx.out();
                }
        };

}   // namespace fmt
