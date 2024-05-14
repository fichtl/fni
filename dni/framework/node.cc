#include "dni/framework/node.h"

#include <memory>
#include <mutex>

#include "dni/framework/datum.h"
#include "dni/framework/default_input_stream_handler.h"
#include "dni/framework/dni.pb.h"
#include "dni/framework/dtype.h"
#include "dni/framework/graph_config.h"
#include "dni/framework/input_stream_handler.h"
#include "dni/framework/input_stream_manager.h"
#include "dni/framework/node_config.h"
#include "dni/framework/output_stream_handler.h"
#include "dni/framework/output_stream_manager.h"
#include "dni/framework/task.h"
#include "dni/framework/task_context.h"
#include "dni/framework/task_state.h"
#include "dni/framework/utils/names.h"
#include "spdlog/spdlog.h"

namespace dni {

        int Node::initializeInputSideData(OutputSideDatumImpl* output_side_data)
        {
                int base = node_info_->input_side_data_base_index;
                for (int i = 0; i < node_info_->Config().InputSideData().size(); ++i)
                {
                        int output_side_data_idx =
                            graph_cfg_->InputSideData()[base + i].upstream_id;
                        if (output_side_data_idx < 0)
                                continue;
                        OutputSideDatumImpl* origin =
                            &output_side_data[output_side_data_idx];
                        origin->AddMirror(&input_side_data_handler_, i);

                        SPDLOG_DEBUG(
                            "OutputSideDatumImpl* origin {}, mirror size: {}",
                            origin->Name(), origin->MirrorSize());
                }
                return 0;
        }

        int Node::initializeOutputSideData(
            const DtypeSet& output_side_data_types, OutputSideDatumImpl* output_side_data)
        {
                output_side_data_ = std::make_unique<OutputSideData>();
                int base = node_info_->output_side_data_base_index;
                for (int i = 0; i < node_info_->Config().OutputSideData().size(); ++i)
                {
                        output_side_data_->emplace_back(&output_side_data[base + i]);
                }

                // debug
                for (int i = 0; i < node_info_->Config().OutputSideData().size(); ++i)
                {
                        auto osd = (OutputSideDatumImpl*) output_side_data_->at(i);
                        SPDLOG_DEBUG(
                            "outside name: {}-----------mirror size: {}", osd->Name(),
                            osd->MirrorSize());
                }

                return 0;
        }

        int Node::initializeInputStream(
            InputStreamManager* input_stream_managers,
            OutputStreamManager* output_stream_managers)
        {
                if (!input_stream_managers || !output_stream_managers)
                        return 0;
                int base = node_info_->input_stream_base_index;
                if (base < 0)
                        return 0;
                SPDLOG_DEBUG(
                    "node {}: input streams range [{},{})", name_, base,
                    base + input_stream_handler_->NStreams());

                input_stream_handler_->InitializeInputStreamManagers(
                    &input_stream_managers[base]);

                // 每个 input_stream_handler_ 都有 InputStreamManagerSet
                // input_stream_managers_; 这里的 i 就表示里面 vector 的索引值， 从 0
                // 算起； AddMirror 表示 input_stream_handler_ 中的 index == i 的
                // InputStreamManager 作为 origin 的 OutputStreamManager 的下游；
                // 因为每个上游的 OutputStreamManager
                // 可能会发到多个下游节点，作为其中一个输入，所以这里需要给每个
                // OutputStreamManager 分配一个 vector 等容器进行记录
                for (int i = 0; i < node_info_->Config().Inputs().size(); ++i)
                {
                        int output_stream_idx =
                            graph_cfg_->InputStreams()[base + i].upstream_id;
                        if (output_stream_idx < 0)
                                continue;
                        OutputStreamManager* origin =
                            &output_stream_managers[output_stream_idx];

                        // SPDLOG_DEBUG(
                        //     "node {}: add to parent {}'s mirrors", name_,
                        //     origin->Name());
                        origin->AddMirror(input_stream_handler_.get(), i);
                        // SPDLOG_DEBUG("node {}: current parent: {}", name_, *origin);
                }
                return 0;
        }

        int Node::initializeOutputStream(OutputStreamManager* output_stream_managers)
        {
                if (!output_stream_managers)
                        return 0;
                int base = node_info_->output_stream_base_index;
                if (base < 0)
                        return 0;
                SPDLOG_DEBUG(
                    "node {}: output streams range [{},{})", name_, base,
                    base + output_stream_handler_->NStreams());

                output_stream_handler_->InitializeOutputStreamManagers(
                    &output_stream_managers[base]);

                return 0;
        }

        int Node::initializeInputStreamHandler(
            const InputStreamHandlerConfig& handler_config,
            const DtypeSet& input_stream_types)
        {
                const auto& name = handler_config.input_stream_handler();
                input_stream_handler_ = GetInputStreamHandlerByName(
                    name, input_stream_types.TagMap(), &context_manager_);
                return 0;
        }

        int Node::initializeOutputStreamHandler(
            const OutputStreamHandlerConfig& handler_config,
            const DtypeSet& output_stream_types)
        {
                const auto& name = handler_config.output_stream_handler();
                output_stream_handler_ = GetOutputStreamHandlerByName(
                    name, output_stream_types.TagMap(), &context_manager_);
                return 0;
        }

        int Node::Initialize(
            const ParsedGraphConfig* graph_config,
            NodeInfo::NodeRef node_ref,
            InputStreamManager* input_stream_managers,
            OutputStreamManager* output_stream_managers,
            OutputSideDatumImpl* output_side_data)
        {
                graph_cfg_ = graph_config;
                node_info_ = &graph_config->Nodes()[node_ref.index];
                const NodeConfig& node_config = node_info_->Config();
                name_ = node_config.NodeName();

                SPDLOG_DEBUG("initializing node {}", name_);

                // * Initialize output streams/sidedata before input streams/sidedata
                // input streams will call `AddMirror` of upstream's output stream
                // managers.

                if (initializeOutputSideData(
                        node_config.OutputSideData(), output_side_data))
                        return -1;
                if (initializeInputSideData(output_side_data))
                        return -1;

                if (initializeOutputStreamHandler(
                        node_config.Proto().output_stream_handler(),
                        node_config.Outputs()))
                        return -1;
                if (initializeOutputStream(output_stream_managers))
                        return -1;

                task_state_ = std::make_unique<TaskState>(
                    name_, node_ref.index, node_config.Proto().task(),
                    node_config.Proto());
                context_manager_.Initialize(
                    task_state_.get(), node_config.Inputs().TagMap(),
                    node_config.Outputs().TagMap(), nproc_ > 1);

                if (initializeInputStreamHandler(
                        node_config.Proto().input_stream_handler(), node_config.Inputs()))
                        return -1;
                if (initializeInputStream(input_stream_managers, output_stream_managers))
                        return -1;

                SPDLOG_DEBUG(
                    "node {}: InputStreamHandler {}, OutputStreamHandler {}", name_,
                    *input_stream_handler_, *output_stream_handler_);

                return 0;
        }

        void Node::InputStreamReady()
        {
                const std::lock_guard<std::mutex> l(status_mu_);
                if (status_ != kStatePrepared)
                        return;
                input_stream_ready_ = true;
        }
        void Node::InputSideDataReady()
        {
                const std::lock_guard<std::mutex> l(status_mu_);
                if (status_ != kStatePrepared)
                        return;
                input_side_data_ready_ = true;
        }

        bool Node::IsReady()
        {
                const std::lock_guard<std::mutex> l(status_mu_);
                return input_stream_ready_ && input_side_data_ready_;
        }

        bool Node::IsSource() const
        {
                return input_stream_handler_->NStreams() == 0 &&
                       output_stream_handler_->NStreams() != 0;
        }

        int Node::PrepareForRun(const std::map<std::string, Datum>& all_side_data)
        {
                // Reset task state
                task_state_->Reset();

                // Prepare input/output stream
                SPDLOG_DEBUG("node {}: preparing input/output stream handlers", name_);
                input_stream_handler_->PrepareForRun();
                output_stream_handler_->PrepareForRun();

                // Prepare input/output side data
                SPDLOG_DEBUG("node {}: preparing input side data", name_);
                input_side_data_types_ =
                    std::make_unique<DtypeSet>(node_info_->Config().InputSideData());
                if (input_side_data_handler_.PrepareForRun(
                        input_side_data_types_.get(), all_side_data))
                {
                        SPDLOG_ERROR("failed to initialize input side data handler");
                        return -1;
                }

                // Prepare task context
                SPDLOG_DEBUG("node {}: preparing task context", name_);
                task_state_->input_side_data = &input_side_data_handler_.Data();
                task_state_->output_side_data = output_side_data_.get();
                context_manager_.PrepareForRun();
                SPDLOG_DEBUG(
                    "node {}: task context manager: {}", name_, context_manager_);
                TaskContext* ctx = context_manager_.DefaultContext();
                if (!ctx)
                {
                        SPDLOG_ERROR("node {}: failed to get default context");
                        return -1;
                }
                SPDLOG_DEBUG("node {}: current context: {}", name_, *ctx);

                SPDLOG_DEBUG("node {}: setup input/output streams", name_);
                input_stream_handler_->SetupInputStreams(&ctx->Inputs());
                output_stream_handler_->SetupOutputStreams(&ctx->Outputs());

                // Prepare task
                SPDLOG_DEBUG("node {}: setup task {}", name_, task_state_->TaskType());
                task_ = GetTaskByName(task_state_->TaskType());

                {
                        const std::lock_guard<std::mutex> l(status_mu_);
                        status_ = kStatePrepared;
                }
                SPDLOG_DEBUG("node {}: status: {}", name_, status_);

                return 0;
        }

        int Node::Open()
        {
                TaskContext* ctx = context_manager_.DefaultContext();

                InputStreamSet& inputs = ctx->Inputs();
                input_stream_handler_->Open(&inputs);

                OutputStreamSet& outputs = ctx->Outputs();
                output_stream_handler_->ResetOutputs(&outputs);

                task_->Open(ctx);

                output_stream_handler_->Open(&outputs);

                {
                        const std::lock_guard<std::mutex> l(status_mu_);
                        status_ = kStateOpened;
                }
                SPDLOG_DEBUG("node {}: status: {}", name_, status_);

                return 0;
        }

        // TODO: rework data pipeline for `taskflow`-based scheduler.
        //
        // Since `taskflow` handles execution sequence among nodes, there is no need to
        // copy/move cache between nodes as MediaPipe does.
        int Node::Process()
        {
                TaskContext* context = context_manager_.DefaultContext();
                // If any input stream is closed (indicated by a timestamp with max value
                // of time_t), this node needs to be closed.
                std::time_t ts = context->NextTimestamp();
                if (TimestampDone(ts))
                {
                        Close();
                        return 0;
                }

                input_stream_handler_->Marshal(&context->Inputs());

                OutputStreamSet& outputs = context->Outputs();
                output_stream_handler_->ResetOutputs(&outputs);

                int ret = task_->Process(context);

                input_stream_handler_->PostProcess(context);
                input_side_data_handler_.PostProcess();
                output_stream_handler_->PostProcess(context);
                // TODO need implement output_side_data_handler_.PostProcess?

                return ret;
        }

        void Node::closeInputStreams()
        {
                {
                        const std::lock_guard<std::mutex> l(status_mu_);
                        if (status_ == kStateClosed)
                                return;
                }
                input_stream_handler_->Close();
        }

        void Node::closeOutputStreams(OutputStreamSet* outputs)
        {
                {
                        const std::lock_guard<std::mutex> l(status_mu_);
                        if (status_ == kStateClosed)
                                return;
                }
                output_stream_handler_->Close(outputs);
        }

        int Node::Close()
        {
                {
                        const std::lock_guard<std::mutex> l(status_mu_);
                        if (status_ == kStateClosed)
                                return 0;
                }

                closeInputStreams();

                TaskContext* ctx = context_manager_.DefaultContext();
                OutputStreamSet& outputs = ctx->Outputs();

                output_stream_handler_->ResetOutputs(&outputs);

                int ret = task_->Close(ctx);

                closeOutputStreams(&outputs);

                {
                        const std::lock_guard<std::mutex> l(status_mu_);
                        status_ = kStateClosed;
                }

                return ret;
        }

        int Node::Finish()
        {
                task_ = nullptr;
                closeInputStreams();
                closeOutputStreams(nullptr);
                {
                        const std::lock_guard<std::mutex> l(status_mu_);
                        status_ = kStateUninitialized;
                }
                return 0;
        }

        // TODO: block by implementation of registry
        std::unique_ptr<InputStreamHandler> GetInputStreamHandlerByName(
            std::string_view name, std::shared_ptr<utils::TagMap> tag_map,
            TaskContextManager* ctx_mngr)
        {
                return std::move(std::make_unique<DefaultInputStreamHandler>(
                    tag_map, ctx_mngr, false));
        }

        // TODO: block by implementation of registry
        std::unique_ptr<OutputStreamHandler> GetOutputStreamHandlerByName(
            std::string_view name, std::shared_ptr<utils::TagMap> tag_map,
            TaskContextManager* ctx_mngr)
        {
                return std::move(
                    std::make_unique<OutputStreamHandler>(tag_map, ctx_mngr, false));
        }

}   // namespace dni
