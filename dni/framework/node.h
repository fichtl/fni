#pragma once

#include <memory>
#include <mutex>

#include "dni/framework/context.h"
#include "dni/framework/datum.h"
#include "dni/framework/dni.pb.h"
#include "dni/framework/dtype.h"
#include "dni/framework/graph_config.h"
#include "dni/framework/input_stream_handler.h"
#include "dni/framework/input_stream_manager.h"
#include "dni/framework/node_config.h"
#include "dni/framework/output_stream_handler.h"
#include "dni/framework/output_stream_manager.h"
#include "dni/framework/task.h"
#include "dni/framework/task_state.h"
#include "fmt/format.h"

namespace dni {

        enum NodeStatus {
                kStateUninitialized = 0,
                kStatePrepared = 1,
                kStateOpened = 2,
                kStateClosed = 3,
        };

        class Node {
        public:
                int Initialize(
                    const ParsedGraphConfig* graph_config,
                    NodeInfo::NodeRef node_ref,
                    InputStreamManager* input_stream_managers,
                    OutputStreamManager* output_stream_managers,
                    OutputSideDatumImpl* output_side_data);

                int Id() const { return node_info_ ? node_info_->Node().index : -1; }

                int PrepareForRun(const std::map<std::string, Datum>& all_side_data);

                void InputStreamReady();
                void InputSideDataReady();
                bool IsReady();

                int Open();

                int Process();

                int Close();

                int Finish();

                const NodeConfig& Config() const { return node_info_->Config(); }

                const std::unordered_set<std::string>& Predecessors() const
                {
                        return node_info_->Predecessors();
                }

                bool IsSource() const;

                const TaskState& GetState() const { return *task_state_; }
                Context* GetContext() const { return context_manager_.DefaultContext(); }

        private:
                int initializeInputSideData(OutputSideDatumImpl* output_side_data);

                int initializeOutputSideData(
                    const DtypeSet& output_side_data_types,
                    OutputSideDatumImpl* output_side_data);

                int initializeInputStream(
                    InputStreamManager* input_stream_managers,
                    OutputStreamManager* output_stream_managers);

                int initializeOutputStream(OutputStreamManager* output_stream_managers);

                int initializeInputStreamHandler(
                    const InputStreamHandlerConfig& handler_config,
                    const DtypeSet& input_stream_types);

                int initializeOutputStreamHandler(
                    const OutputStreamHandlerConfig& handler_config,
                    const DtypeSet& output_stream_types);

                void closeInputStreams();

                void closeOutputStreams(OutputStreamSet* outputs);

                std::string name_;

                // Control number of tasks in parallel.
                int nproc_ = 1;

                std::mutex status_mu_;
                NodeStatus status_ = NodeStatus::kStateUninitialized;

                std::unique_ptr<TaskBase> task_;
                std::unique_ptr<TaskState> task_state_;

                ContextManager context_manager_;

                std::unique_ptr<DtypeSet> input_side_data_types_;
                InputSideDataHandler input_side_data_handler_;
                bool input_side_data_ready_;

                std::unique_ptr<InputStreamHandler> input_stream_handler_;
                bool input_stream_ready_;

                std::unique_ptr<OutputSideData> output_side_data_;

                std::unique_ptr<OutputStreamHandler> output_stream_handler_;

                const ParsedGraphConfig* graph_cfg_ = nullptr;
                const NodeInfo* node_info_ = nullptr;

                friend class fmt::formatter<dni::Node>;
                friend class fmt::formatter<std::unique_ptr<dni::Node>>;
        };

}   // namespace dni

namespace fmt {

        template <>
        struct formatter<dni::NodeStatus>: formatter<std::string_view> {
                auto format(const dni::NodeStatus& st, format_context& ctx) const
                {
                        switch (st)
                        {
                        case dni::NodeStatus::kStateUninitialized:
                                return format_to(ctx.out(), "UNINITIALIZED");
                        case dni::NodeStatus::kStatePrepared:
                                return format_to(ctx.out(), "PREPARED");
                        case dni::NodeStatus::kStateOpened:
                                return format_to(ctx.out(), "OPENED");
                        case dni::NodeStatus::kStateClosed:
                                return format_to(ctx.out(), "CLOSED");
                        default: return format_to(ctx.out(), "UNEXPECTED");
                        }
                }
        };

        template <>
        struct formatter<dni::Node>: formatter<std::string_view> {
                auto format(const dni::Node& node, format_context& ctx) const
                {
                        return format_to(
                            ctx.out(), "node {}: task({}), {}, {}", node.name_,
                            node.Config().Proto().task(), *node.input_stream_handler_,
                            *node.output_stream_handler_);
                }
        };
        template <>
        struct formatter<std::unique_ptr<dni::Node>>: formatter<std::string_view> {
                auto format(
                    const std::unique_ptr<dni::Node>& node, format_context& ctx) const
                {
                        return format_to(
                            ctx.out(), "node {}: task({}), {}, {}", node->name_,
                            node->Config().Proto().task(), *node->input_stream_handler_,
                            *node->output_stream_handler_);
                }
        };

}   // namespace fmt
