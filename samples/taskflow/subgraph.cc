#include <iostream>

#include "taskflow/taskflow.hpp"

int main(int argc, char** argv)
{
        tf::Executor executor;
        tf::Taskflow taskflow;

        tf::Task A = taskflow.emplace([]() { std::cout << "TaskA\n"; }).name("A");
        tf::Task C = taskflow.emplace([]() { std::cout << "TaskC\n"; }).name("C");
        tf::Task D = taskflow.emplace([]() { std::cout << "TaskD\n"; }).name("D");

        tf::Task B = taskflow.emplace([](tf::Subflow& subflow) {
                tf::Task B1 =
                    subflow.emplace([]() { std::cout << "TaskB1\n"; }).name("B1");
                tf::Task B2 =
                    subflow.emplace([]() { std::cout << "TaskB2\n"; }).name("B2");
                tf::Task B3 =
                    subflow.emplace([]() { std::cout << "TaskB3\n"; }).name("B3");
                B3.succeed(B1, B2);   // B3 runs after B1 and B2
        });

        A.precede(B, C);
        D.succeed(B, C);

        executor.run(taskflow).wait();

        return 0;
}