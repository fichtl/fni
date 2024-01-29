#pragma once

#include <memory>
#include <mutex>

#include "dni/framework/context.h"
#include "dni/framework/datum.h"
#include "dni/framework/datum_type.h"
#include "dni/framework/graph_config.h"
#include "dni/framework/input_stream_handler.h"
#include "dni/framework/input_stream_manager.h"
#include "dni/framework/node_config.h"
#include "dni/framework/output_stream_handler.h"
#include "dni/framework/output_stream_manager.h"
#include "dni/framework/task.h"
#include "dni/framework/task_state.h"

namespace dni {

        enum class NodeReadiness {
                kNotReady,
                kReadyForProcess,
                kReadyForClose,
        };

        enum class NodeStatus {
                kStateUninitialized = 0,
                kStatePrepared = 1,
                kStateOpened = 2,
                kStateClosed = 3,
        };

        class Node {
        public:
                Node();

                int Initialize(
                    const ValidatedGraphConfig* graph,
                    NodeInfo::NodeRef node_ref,
                    InputStreamManager* input_stream_managers,
                    OutputStreamManager* output_stream_managers,
                    OutputSideData* output_side_data);

                int Id() const { return node_info_ ? node_info_->Node().index : -1; }

                int PrepareForRun(const std::map<std::string, Datum>& all_side_data);

                void InputStreamHeadersReady();
                void InputSideDataReady();
                void OutputStreamHeadersReady();
                void OutputSideDataReady();

                int OpenTask();

                int ProcessTask(Context* cnotext);

                int CloseTask();

                const NodeConfig& Config() const { return node_info_->Config(); }

                bool IsSource() const
                {
                        return input_stream_handler_->NStreams() == 0 &&
                               output_stream_handler_->NStreams() != 0;
                }

        private:
                int InitializeInputSideData(OutputSideDatumImpl* output_side_data_impl);

                int InitializeOutputSideData(
                    const DatumTypeSet& output_side_data_types,
                    OutputSideDatumImpl* output_side_data_impl);

                int InitializeInputStream(OutputStreamManager* output_stream_managers);

                int InitializeOutputStream(
                    InputStreamManager* input_stream_managers,
                    OutputStreamManager* output_stream_managers);

                int InitializeInputStreamHandler(
                    const InputStreamHandlerConfig& handler_config,
                    const DatumTypeSet& input_stream_types);

                int InitializeOutputStreamHandler(
                    const OutputStreamHandlerConfig& handler_config,
                    const DatumTypeSet& output_stream_types);

                std::string name_;

                NodeStatus status_ = NodeStatus::kStateUninitialized;

                std::unique_ptr<TaskBase> task_;
                std::unique_ptr<TaskState> task_state_;

                ContextManager context_manager_;

                std::mutex mu_;

                std::unique_ptr<DatumTypeSet> input_side_data_types_;

                InputSideDataHandler input_side_data_handler_;

                std::unique_ptr<OutputSideData> output_side_data_;

                std::unique_ptr<InputStreamHandler> input_stream_handler_;

                std::unique_ptr<OutputStreamHandler> output_stream_handler_;

                const ValidatedGraphConfig* graph_cfg_ = nullptr;
                const NodeInfo* node_info_ = nullptr;
        };

}   // namespace dni
