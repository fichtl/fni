#include "dni/framework/output_stream_handler.h"

#include <memory>

#include "dni/framework/context_manager.h"
#include "dni/framework/output_stream.h"
#include "dni/framework/output_stream_manager.h"
#include "dni/framework/utils/tags.h"
#include "spdlog/spdlog.h"

namespace dni {

        OutputStreamHandler::OutputStreamHandler(
            std::shared_ptr<utils::TagMap> tag_map, ContextManager* context_manager,
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
        }

        void OutputStreamHandler::PrepareForRun()
        {
                for (const auto& manager : output_stream_managers_)
                {
                        manager->PrepareForRun();
                }
        }

        void OutputStreamHandler::Propagate(OutputStreamSet* outputs)
        {
                for (size_t i = 0; i < outputs->size(); i++)
                {
                        OutputStreamManager* manager = output_stream_managers_[i];
                        // TODO: If the stream is the last element in mirrors_, moves
                        // packets from output_queue_. Otherwise, copies the packets.
                        for (auto& m : manager->Mirrors())
                        {
                                m.ish->AddData(m.id, *outputs->at(i).OutputQueue());
                        }
                }
        }

        void OutputStreamHandler::PostProcess(Context* context)
        {
                Propagate(&context->Outputs());
        }

        // TODO: return output stream handler by name.
        std::unique_ptr<OutputStreamHandler> GetOutputStreamHandlerByName(
            std::string_view name, std::shared_ptr<utils::TagMap> tag_map,
            ContextManager* ctx_mngr)
        {
                return std::move(
                    std::make_unique<OutputStreamHandler>(tag_map, ctx_mngr, false));
        }

}   // namespace dni
