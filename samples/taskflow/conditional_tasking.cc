#include <iostream>

#include "taskflow/taskflow.hpp"

int main(int argc, char** argv)
{
        tf::Taskflow taskflow;
        tf::Executor executor;

        auto [init, cond, yes, no] = taskflow.emplace(
            []() {},
            []() { return 0; },
            []() { std::cout << "yes\n"; },
            []() { std::cout << "no\n"; });

        cond.succeed(init).precede(yes, no);

        executor.run(taskflow).wait();

        return 0;
}
