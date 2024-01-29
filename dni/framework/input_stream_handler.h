#pragma once

#include <ctime>
#include <list>
#include <memory>

#include "dni/framework/context_manager.h"
#include "dni/framework/datum.h"
#include "dni/framework/input_stream_manager.h"
#include "dni/framework/node.h"
#include "dni/framework/utils/tags.h"

namespace dni {

        class InputStreamHandler {
        public:
                InputStreamHandler(
                    std::shared_ptr<utils::TagMap> tag_map,
                    ContextManager* context_manager, bool in_parallel);

                int InitializeInputStreamManagers(
                    InputStreamManager* input_stream_managers_arr);

                int SetupInputStreams();

                // Resets the input stream handler and its underlying input streams for
                // another run of the graph.
                virtual void PrepareForRun();

                // Add data into InputStreamManager's queue.
                virtual void AddData(int id, const std::list<Datum>& data);

                virtual void MoveData(int id, const std::list<Datum>* data);

                void SetNextTimestampBound(int id, std::time_t ts);

                void Close();

                bool Schedule(int max, std::time_t bound);

                int NStreams() { return input_stream_managers_.size(); }

                class SyncSet {
                public:
                        SyncSet(
                            InputStreamHandler* input_stream_handler,
                            std::vector<int>
                                ids);

                        void PrepareForRun();

                        NodeReadiness GetNodeReadiness(std::time_t ts);

                        void SyncByTime(std::time_t ts, InputStreamSet* input_dataset);

                        void Sync(InputStreamSet* input_dataset);

                private:
                        InputStreamHandler* input_stream_handler_;
                        std::vector<int> ids_;
                        std::time_t last_processed_ = 0;
                };

        protected:
                virtual NodeReadiness GetNodeReadiness(std::time_t ts);

                // Add data into a specific InputStreamImpl instance.
                void marshal(InputStreamImpl* input, Datum&& datum, bool done)
                {
                        input->AddDatum(std::move(datum), done);
                }

                virtual void Marshal(InputStreamSet* inputs);

                InputStreamManagerSet input_stream_managers_;
                ContextManager* const context_manager_;
                const bool in_parallel_;
        };

}   // namespace dni
