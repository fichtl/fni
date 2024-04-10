#include "dni/framework/graph_output_stream.h"

#include <memory>

#include "dni/framework/input_stream_manager.h"
#include "dni/framework/output_stream_manager.h"
#include "dni/framework/utils/tags.h"
#include "spdlog/spdlog.h"

namespace dni {

        int GraphOutputStream::Initialize(
            const std::string& name, OutputStreamManager* osm)
        {
                google::protobuf::RepeatedPtrField<std::string> stream_name;
                stream_name.Add()->assign(name);
                std::shared_ptr<utils::TagMap> input_tags = utils::NewTagMap(stream_name);
                // should not happened
                if (!input_tags)
                {
                        SPDLOG_ERROR("invalid GraphOutputStream: {:}", stream_name);
                        return -1;
                }
                input_stream_handler_ = std::make_unique<GraphOutputStreamHandler>(
                    input_tags, nullptr, false);
                input_stream_manager_ = std::make_unique<InputStreamManager>();
                input_stream_handler_->InitializeInputStreamManagers(
                    input_stream_manager_.get());
                input_stream_manager_->Initialize(name, nullptr);
                osm->AddMirror(input_stream_handler_.get(), 0);

                return 0;
        }

        void GraphOutputStream::PrepareForRun()
        {
                input_stream_handler_->PrepareForRun();
        }

        void GraphOutputStream::Close() { input_stream_manager_->Close(); }

}   // namespace dni
