#include "dni/framework/task.h"

#include "dni/framework/register.h"

namespace dni {

        std::unique_ptr<TaskBase> GetTaskByName(const std::string& task_type)
        {
                return TaskFactory::get()->produce_unique(task_type);
        }

}   // namespace dni
