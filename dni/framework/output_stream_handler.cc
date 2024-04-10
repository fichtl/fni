#include "dni/framework/output_stream_handler.h"

#include <memory>

#include "dni/framework/output_stream.h"
#include "dni/framework/output_stream_manager.h"
#include "dni/framework/task_context_manager.h"
#include "dni/framework/utils/tags.h"
#include "spdlog/spdlog.h"

namespace dni {

        OutputStreamHandler::OutputStreamHandler(
            std::shared_ptr<utils::TagMap> tag_map, TaskContextManager* context_manager,
            bool in_parallel)
            : tag_map_(tag_map), context_manager_(context_manager),
              in_parallel_(in_parallel)
        {
                output_stream_managers_ =
                    std::vector<OutputStreamManager*>(tag_map_->NStreams());
        }

        int OutputStreamHandler::InitializeOutputStreamManagers(
            OutputStreamManager* managers_arr)
        {
                for (int i = 0; i < output_stream_managers_.size(); ++i)
                {
                        output_stream_managers_[i] = &managers_arr[i];
                };
                return 0;
        }

        int OutputStreamHandler::SetupOutputStreams(OutputStreamSet* outputs)
        {
                for (int i = 0; i < output_stream_managers_.size(); ++i)
                {
                        outputs->at(i).SetSpec(output_stream_managers_[i]->Spec());
                }
                return 0;
        }

        void OutputStreamHandler::PrepareForRun()
        {
                for (const auto& manager : output_stream_managers_)
                {
                        manager->PrepareForRun();
                }
        }

        // TODO:
        void OutputStreamHandler::ResetOutputs(OutputStreamSet* outputs) {}

        // TODO:
        void OutputStreamHandler::Open(OutputStreamSet* outputs) {}

        // TODO: propagate timestamp as well.
        void OutputStreamHandler::Propagate(OutputStreamSet* outputs)
        {
                for (size_t i = 0; i < outputs->size(); i++)
                {
                        OutputStreamManager* manager = output_stream_managers_[i];
                        std::list<Datum>* output_data = outputs->at(i).OutputQueue();
                        int nmirrors = manager->Mirrors().size();
                        for (int i = 0; i < nmirrors; ++i)
                        {
                                const auto& m = manager->Mirrors()[i];
                                if (i == nmirrors - 1)
                                {
                                        m.ish->MoveData(m.id, output_data);
                                }
                                else
                                {
                                        m.ish->AddData(m.id, *output_data);
                                }
                        }
                        output_data->clear();
                }
        }

        void OutputStreamHandler::PostProcess(TaskContext* context)
        {
                Propagate(&context->Outputs());
        }

        // TODO:
        void OutputStreamHandler::Close(OutputStreamSet* outputs) {}

}   // namespace dni
