#pragma once

#include <chrono>
#include <iostream>
#include <string>
#include <string_view>
#include <thread>

#include "dni/framework/task_context.h"

namespace dni {

        class TaskBase {
        public:
                TaskBase() {}
                virtual ~TaskBase() {}

                virtual int Open(TaskContext* ctx) { return 0; };

                virtual int Process(TaskContext* ctx) = 0;

                virtual int Close(TaskContext* ctx) { return 0; };
        };

        std::unique_ptr<TaskBase> GetTaskByName(const std::string& name);

}   // namespace dni
