#include "taskflow/taskflow.hpp"

int main(int argc, char** argv)
{
        tf::Taskflow taskflow;
        tf::Executor executor;

        taskflow.emplace([&](tf::Runtime& rt) { assert(&(rt.executor()) == &executor); });

        executor.run(taskflow).wait();
}
