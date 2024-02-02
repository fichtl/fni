#pragma once

#include <chrono>
#include <iostream>
#include <string>
#include <string_view>
#include <thread>

#include "dni/framework/context.h"

namespace dni {

        class TaskBase {
        public:
                TaskBase() {}
                virtual ~TaskBase() {}

                virtual int Open(Context* ctx) { return 0; };

                virtual int Process(Context* ctx) = 0;

                virtual int Close(Context* ctx) { return 0; };
        };

        std::unique_ptr<TaskBase> GetTaskByName(const std::string& name);

}   // namespace dni
