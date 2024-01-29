#pragma once

#include <list>
#include <mutex>
#include <string>
#include <vector>

#include "dni/framework/datum.h"
#include "dni/framework/datum_type.h"
#include "dni/framework/input_stream_handler.h"

namespace dni {

        class OutputStreamManager {
        public:
                OutputStreamManager() = default;
                OutputStreamManager(const OutputStreamManager&) = delete;
                OutputStreamManager& operator=(const OutputStreamManager&) = delete;

                int Initialize(const std::string& name, const DatumType* type);

                void PrepareForRun();

                void AddMirror(InputStreamHandler* input_stream_handler, int id);

                void Close();

        private:
                struct Mirror {
                        Mirror(InputStreamHandler* input_stream_handler, int id)
                            : input_stream_handler(input_stream_handler), id(id)
                        {}
                        InputStreamHandler* input_stream_handler;
                        int id;
                };

                std::string name_;
                std::mutex mu_;

                std::vector<Mirror> mirrors_;
        };

        using OutputStreamManagerSet = std::set<OutputStreamManager*>;

}   // namespace dni
