#pragma once

#include <memory>

#include "dni/framework/input_stream_handler.h"
#include "dni/framework/input_stream_manager.h"
#include "dni/framework/output_stream_manager.h"

namespace dni {

        class GraphOutputStream {
        public:
                int Initialize(const std::string& name, OutputStreamManager* osm);

                void PrepareForRun();

                void Close();

                const std::string& Name() { return input_stream_manager_->Name(); }

                Datum Head() const { return input_stream_manager_->Head(); }

                void Clear() { input_stream_manager_->Clear(); }

        protected:
                class GraphOutputStreamHandler: public InputStreamHandler {
                public:
                        GraphOutputStreamHandler(
                            std::shared_ptr<utils::TagMap> tag_map,
                            TaskContextManager* context_manager, bool in_parallel)
                            : InputStreamHandler(
                                  std::move(tag_map), context_manager, in_parallel)
                        {}

                        DataReadiness GetReadiness(std::time_t* ts) override {}

                        void Marshal(InputStreamSet* inputs) override {}
                };

        private:
                std::unique_ptr<InputStreamHandler> input_stream_handler_;
                std::unique_ptr<InputStreamManager> input_stream_manager_;
        };

}   // namespace dni
