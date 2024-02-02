#pragma once

#include <ctime>
#include <memory>
#include <queue>

#include "dni/framework/datum.h"
#include "dni/framework/input_side_data.h"
#include "dni/framework/input_stream.h"
#include "dni/framework/output_side_data.h"
#include "dni/framework/output_stream.h"
#include "dni/framework/task_state.h"
#include "dni/framework/timestamp.h"
#include "dni/framework/utils/tags.h"
#include "fmt/format.h"

namespace dni {

        class Context {
        public:
                Context(
                    TaskState* state, std::shared_ptr<utils::TagMap> input_tags,
                    std::shared_ptr<utils::TagMap> output_tags)
                    : state_(state)
                {
                        inputs_ = MakeInputStreamSetFromTagMap(std::move(input_tags));
                        outputs_ = MakeOutputStreamSetFromTagMap(std::move(output_tags));
                }

                const std::string& Name() const { return state_->NodeName(); }

                int Id() const { return state_->NodeId(); }

                const std::string& Type() const { return state_->TaskType(); }

                const InputSideData& GetInputSideData() const
                {
                        return *state_->input_side_data;
                }

                OutputSideData& GetOutputSideData() { return *state_->output_side_data; }

                InputStreamSet& Inputs() { return inputs_; }
                const InputStreamSet& Inputs() const { return inputs_; }

                OutputStreamSet& Outputs() { return outputs_; }
                const OutputStreamSet& Outputs() const { return outputs_; }

                void PushTimestamp(std::time_t ts) { timestamps_.push(ts); }
                void PopTimestamp()
                {
                        if (timestamps_.empty())
                                return;
                        timestamps_.pop();
                }
                std::time_t NextTimestamp() const
                {
                        return timestamps_.empty() ? kCtimeMin : timestamps_.front();
                }

        private:
                TaskState* state_;

                // TODO: merge into Input/OutputStreamSet
                std::shared_ptr<utils::TagMap> input_tag_map_;
                std::shared_ptr<utils::TagMap> output_tag_map_;

                InputStreamSet inputs_;
                OutputStreamSet outputs_;

                std::queue<std::time_t> timestamps_;

                friend class ContextManager;
                friend class fmt::formatter<dni::Context>;
                friend class fmt::formatter<std::unique_ptr<dni::Context>>;
        };

}   // namespace dni

namespace fmt {

        template <>
        struct formatter<std::unique_ptr<dni::Context>>: formatter<std::string_view> {
                auto format(
                    const std::unique_ptr<dni::Context>& c, format_context& ctx) const
                {
                        return format_to(
                            ctx.out(), "in({})/out({})", c->inputs_.size(),
                            c->outputs_.size());
                }
        };
        template <>
        struct formatter<dni::Context>: formatter<std::string_view> {
                auto format(const dni::Context& c, format_context& ctx) const
                {
                        return format_to(
                            ctx.out(), "in({})/out({})", c.inputs_.size(),
                            c.outputs_.size());
                }
        };

}   // namespace fmt
