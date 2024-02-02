#include <iostream>

#include "taskflow/taskflow.hpp"

int main(int argc, char** argv)
{
        tf::Executor executor;
        tf::Taskflow taskflow;

        tf::Task A = taskflow.placeholder();
        tf::Task B = taskflow.emplace([]() { std::cout << "TaskB\n"; }).name("TaskB");

        auto [D, E, F] = taskflow.emplace(
            []() { std::cout << "TaskD\n"; },
            []() { std::cout << "TaskE\n"; },
            []() { std::cout << "TaskF\n"; });

        A.name("TaskA");
        A.work([A]() { std::cout << A.name() << std::endl; });
        A.precede(B);
        A.precede(D);
        B.precede(F);
        D.precede(F);

        executor.run(taskflow).wait();

        std::cout << "-----\n";

        std::cout << A.name() << std::endl;
        std::cout << A.num_successors() << std::endl;
        std::cout << A.num_dependents() << std::endl;

        std::cout << B.name() << std::endl;
        std::cout << B.num_successors() << std::endl;
        std::cout << B.num_dependents() << std::endl;

        std::cout << "-----\n";

        taskflow.dump(std::cout);

        return 0;
}
