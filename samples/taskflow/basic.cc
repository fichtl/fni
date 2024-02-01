#include <iostream>

#include "taskflow/taskflow.hpp"

int main(int argc, char** argv)
{
        tf::Executor executor;
        tf::Taskflow taskflow;

        auto [A, B, C, D] = taskflow.emplace(
            []() { std::cout << "TaskA\n"; }, []() { std::cout << "TaskB\n"; },
            []() { std::cout << "TaskC\n"; }, []() { std::cout << "TaskD\n"; });

        A.precede(B, C);
        D.succeed(B, C);

        executor.run(taskflow).wait();

        return 0;
}
