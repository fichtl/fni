#pragma once

#include <memory>
#include <string_view>

#include "dni/framework/output_stream.h"
#include "dni/framework/output_stream_manager.h"
#include "dni/framework/task_context.h"
#include "dni/framework/task_context_manager.h"
#include "dni/framework/utils/tags.h"
#include "fmt/format.h"

namespace dni {

        class OutputStreamHandler {
        public:
                OutputStreamHandler(
                    std::shared_ptr<utils::TagMap> tag_map,
                    TaskContextManager* context_manager, bool in_parallel);

                OutputStreamHandler(OutputStreamHandler&&) = default;

                int InitializeOutputStreamManagers(OutputStreamManager* managers_arr);

                int SetupOutputStreams(OutputStreamSet* outputs);

                void PrepareForRun();

                void ResetOutputs(OutputStreamSet* outputs);

                void Open(OutputStreamSet* outputs);

                void Propagate(OutputStreamSet* outputs);

                void PostProcess(TaskContext* context);

                void Close(OutputStreamSet* outputs);

                int NStreams() { return output_stream_managers_.size(); }

        protected:
                OutputStreamManagerSet output_stream_managers_;
                std::shared_ptr<utils::TagMap> tag_map_;
                TaskContextManager* const context_manager_;
                const bool in_parallel_;

                friend class fmt::formatter<dni::OutputStreamHandler>;
        };

}   // namespace dni

namespace fmt {

        template <>
        struct formatter<dni::OutputStreamHandler>: formatter<std::string_view> {
                auto format(
                    const dni::OutputStreamHandler& hdl, format_context& ctx) const
                {
                        return format_to(
                            ctx.out(), "OSH({:p},managers[{:}])", fmt::ptr(&hdl),
                            fmt::join(hdl.output_stream_managers_, ","));
                }
        };

}   // namespace fmt
