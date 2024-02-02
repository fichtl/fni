#pragma once

#include <memory>

#include "dni/framework/default_input_stream_handler.h"
#include "dni/framework/input_stream_handler.h"
#include "dni/framework/input_stream_manager.h"
#include "dni/framework/output_stream_manager.h"

namespace dni {

        class GraphOutputStream {
        public:
                GraphOutputStream()
                {
                        input_stream_handler_ =
                            std::make_unique<DefaultInputStreamHandler>(1);
                        input_stream_manager_ = std::make_unique<InputStreamManager>();
                        input_stream_handler_->InitializeInputStreamManagers(
                            input_stream_manager_.get());
                }

                void Initialize(const std::string& name, OutputStreamManager* osm)
                {
                        input_stream_manager_->Initialize(name, nullptr);
                        osm->AddMirror(input_stream_handler_.get(), 0);
                }

                template <typename T>
                inline const T& Value() const
                {
                        return input_stream_manager_->Head().Value<T>();
                }

                // TODO: don't understand the purpose of this func.
                // void PopAfterGet() { input_stream_manager_->Pop(); }

                InputStreamManager* GetInputStreamManager()
                {
                        return input_stream_manager_.get();
                }

                void PrepareForRun() { input_stream_handler_->PrepareForRun(); }

                void Close() { input_stream_manager_->Close(); }

        private:
                std::unique_ptr<InputStreamHandler> input_stream_handler_;
                std::unique_ptr<InputStreamManager> input_stream_manager_;
        };

}   // namespace dni
