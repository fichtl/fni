#include "dni/framework/input_stream_handler.h"

#include <algorithm>
#include <ctime>
#include <list>
#include <memory>

#include "dni/framework/datum.h"
#include "dni/framework/input_stream_manager.h"
#include "dni/framework/task_context_manager.h"
#include "dni/framework/utils/tags.h"
#include "spdlog/spdlog.h"

namespace dni {

        InputStreamHandler::InputStreamHandler(int managers_count)
            : tag_map_(nullptr), context_manager_(nullptr), in_parallel_(false)
        {
                input_stream_managers_ = std::vector<InputStreamManager*>(managers_count);
        }

        InputStreamHandler::InputStreamHandler(
            std::shared_ptr<utils::TagMap> tag_map, TaskContextManager* context_manager,
            bool in_parallel)
            : tag_map_(tag_map), context_manager_(context_manager),
              in_parallel_(in_parallel)
        {
                input_stream_managers_ =
                    std::vector<InputStreamManager*>(tag_map_->NStreams());
        }

        int InputStreamHandler::InitializeInputStreamManagers(
            InputStreamManager* input_stream_managers)
        {
                for (int i = 0; i < input_stream_managers_.size(); ++i)
                {
                        input_stream_managers_[i] = &input_stream_managers[i];
                }
                return 0;
        }

        int InputStreamHandler::SetupInputStreams(InputStreamSet* inputs)
        {
                if (inputs == nullptr)
                        return 0;
                for (int i = 0; i < input_stream_managers_.size(); ++i)
                {
                        // TODO: setup the i-th input stream
                }
                return 0;
        }

        void InputStreamHandler::PrepareForRun()
        {
                for (const auto& manager : input_stream_managers_)
                {
                        manager->PrepareForRun();
                }
        }

        void InputStreamHandler::Open(InputStreamSet* inputs)
        {
                for (int i = 0; i < input_stream_managers_.size(); ++i)
                {
                        // TODO: open the i-th input stream
                }
        }

        void InputStreamHandler::AddData(int id, const std::list<Datum>& data)
        {
                input_stream_managers_[id]->AddData(data);
        }

        void InputStreamHandler::MoveData(int id, std::list<Datum>* data)
        {
                input_stream_managers_[id]->MoveData(data);
        }

        void InputStreamHandler::SetNextTimestampBound(int id, std::time_t ts)
        {
                return input_stream_managers_[id]->SetNextTimestampBound(ts);
        }

        bool InputStreamHandler::Process(std::time_t* bound)
        {
                if (input_stream_managers_.empty())
                        return true;
                std::time_t min_ts = kCtimeMax;
                DataReadiness readiness = GetReadiness(&min_ts);
                switch (readiness)
                {
                // TODO: data is invalid, go back to the first node/task of the graph
                case DataReadiness::kNotReady: {
                        *bound = min_ts;
                        break;
                }
                // TODO: proceed to task body
                case DataReadiness::kReadyForProcess: {
                        TaskContext* ctx = context_manager_->DefaultContext();
                        ctx->PushTimestamp(min_ts);
                        break;
                }
                case DataReadiness::kReadyForClose: {
                        return false;
                }
                }
                return true;
        }

        void InputStreamHandler::PostProcess(TaskContext* context)
        {
                context->PopTimestamp();
                for (auto& input : context->Inputs())
                {
                        input.Clear();
                }
        }

        void InputStreamHandler::Close()
        {
                for (auto& manager : input_stream_managers_)
                {
                        manager->Close();
                }
        }

        void InputStreamHandler::SyncSet::PrepareForRun() { last_processed_ = kCtimeMin; }

        DataReadiness InputStreamHandler::SyncSet::GetReadiness(std::time_t* ts)
        {
                std::time_t min_bound = kCtimeMax;
                std::time_t min_datum = kCtimeMax;
                for (int id : ids_)
                {
                        auto& manager = input_stream_handler_->input_stream_managers_[id];
                        std::time_t curr = manager->NextTimestampOrBound();
                        if (manager->IsEmpty())
                        {
                                min_bound = std::min(curr, min_bound);
                        }
                        else
                        {
                                min_datum = std::min(curr, min_datum);
                        }
                }
                *ts = std::min(min_bound, min_datum);
                if (*ts >= TimestampEnd())
                {
                        *ts = kCtimeMax;
                        last_processed_ = TimestampEnd();
                        return DataReadiness::kReadyForClose;
                }
                if (min_bound > min_datum)
                {
                        last_processed_ = *ts;
                        return DataReadiness::kReadyForProcess;
                }
                return DataReadiness::kNotReady;
        }

        void InputStreamHandler::SyncSet::SyncAt(
            std::time_t ts, InputStreamSet* input_dataset)
        {
                for (int i = 0; i < input_dataset->size(); i++)
                {
                        int ndropped = 0;
                        bool done = false;
                        Datum d = input_stream_handler_->input_stream_managers_[i]->PopAt(
                            ts, &ndropped, &done);
                        if (done)
                        {
                                SPDLOG_DEBUG("InputStream is done or not empty");
                        }
                        input_stream_handler_->marshal(
                            &input_dataset->at(i), std::move(d), done);
                }
        }

        void InputStreamHandler::SyncSet::Sync(InputStreamSet* input_dataset)
        {
                for (int i = 0; i < input_dataset->size(); i++)
                {
                        auto& manager = input_stream_handler_->input_stream_managers_[i];
                        std::time_t ts = manager->NextTimestampOrBound();
                        bool done = false;
                        while (!manager->IsEmpty())
                        {
                                Datum d = manager->Pop(&done);
                                if (done)
                                {
                                        SPDLOG_DEBUG("InputStream is done or not empty");
                                        break;
                                }
                                input_stream_handler_->marshal(
                                    &input_dataset->at(i), d.At(ts - 1), done);
                        }
                }
        }

}   // namespace dni
