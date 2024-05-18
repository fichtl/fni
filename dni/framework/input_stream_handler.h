#pragma once

#include <ctime>
#include <iostream>
#include <list>
#include <memory>

#include "dni/framework/datum.h"
#include "dni/framework/input_stream_manager.h"
#include "dni/framework/task_context.h"
#include "dni/framework/task_context_manager.h"
#include "dni/framework/utils/tags.h"
#include "fmt/format.h"

namespace dni {

        enum class DataReadiness {
                kNotReady,
                kReadyForProcess,
                kReadyForClose,
        };

        class InputStreamHandler {
        public:
                InputStreamHandler(int managers_count);

                InputStreamHandler(
                    std::shared_ptr<utils::TagMap> tag_map,
                    TaskContextManager* context_manager, bool in_parallel);

                int InitializeInputStreamManagers(InputStreamManager* managers_arr);

                int SetupInputStreams(InputStreamSet* inputs);

                // Resets the input stream handler and its underlying input streams for
                // another run of the graph.
                virtual void PrepareForRun();

                void Open(InputStreamSet* inputs);

                // Add data into InputStreamManager's queue.
                virtual void AddData(int id, const std::list<Datum>& data);

                virtual void MoveData(int id, std::list<Datum>* data);

                void SetNextTimestampBound(int id, std::time_t ts);

                // Conditional task that validates input data, marshal data to input
                // stream impl, and mark the timestamp bound accordingly. We use `bound`
                // to indicate the readiness of data. If input data is invalid, set bound
                // to kCtimeMin, otherwise it's set to a valid timestamp.
                bool Process(std::time_t* bound);

                void PostProcess(TaskContext* context);

                void Close();

                int NStreams() { return input_stream_managers_.size(); }

                std::shared_ptr<utils::TagMap> GetTagMap() { return tag_map_; }

                class SyncSet {
                public:
                        SyncSet(
                            InputStreamHandler* input_stream_handler,
                            std::vector<int>
                                ids)
                            : input_stream_handler_(input_stream_handler),
                              ids_(std::move(ids)) {};

                        void PrepareForRun();

                        DataReadiness GetReadiness(std::time_t* ts);

                        // Marshal data at timestamp TS from `InputStreamManager`s' queues
                        // into the `InputStreamSet` INPUTS.
                        void SyncAt(std::time_t ts, InputStreamSet* inputs);

                        // Marshal data from `InputStreamManager`s' queues into
                        // the `InputStreamSet` INPUTS.
                        void Sync(InputStreamSet* inputs);

                private:
                        InputStreamHandler* input_stream_handler_;
                        std::vector<int> ids_;   // list of ids of input streams
                        std::time_t last_processed_ = 0;
                };

                virtual DataReadiness GetReadiness(std::time_t* ts) = 0;

                // Marshal data from `InputStreamManager`s' queues into corresponding
                // `InputStreamImpl`s.
                virtual void Marshal(InputStreamSet* inputs) = 0;

        protected:
                // Add/Move data into the InputStreamImpl.
                void marshal(InputStreamImpl* input, Datum&& datum, bool done)
                {
                        input->AddDatum(std::move(datum), done);
                }

        private:
                InputStreamManagerSet input_stream_managers_;
                std::shared_ptr<utils::TagMap> tag_map_;
                // TODO: consider remove this.
                // OperatorContextManager is only used when checking data readiness, which
                // is not necessary in this architecture.
                TaskContextManager* const context_manager_;
                const bool in_parallel_;

                friend class fmt::formatter<dni::InputStreamHandler>;
        };

}   // namespace dni

namespace fmt {

        template <>
        struct formatter<dni::InputStreamHandler>: formatter<std::string_view> {
                auto format(const dni::InputStreamHandler& hdl, format_context& ctx) const
                {
                        return format_to(
                            ctx.out(), "ISH({:p},managers[{}])", fmt::ptr(&hdl),
                            fmt::join(hdl.input_stream_managers_, ","));
                }
        };

}   // namespace fmt
