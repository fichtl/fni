#pragma once

#include "dni/framework/context.h"

namespace dni {
        class TaskBase {
                TaskBase();
                virtual ~TaskBase();

                virtual int Open(Context* ctx);

                virtual int Process(Context* ctx);

                virtual int Close(Context* ctx);
        };
}   // namespace dni
