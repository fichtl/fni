#include <iostream>
#include <unistd.h>

#include "taskflow/taskflow.hpp"

int main(int argc, char** argv)
{
        tf::Executor executor;

        tf::Taskflow f1, f2;

        // create taskflow f1 of two tasks
        tf::Task f1A = f1.emplace([]() {
                                 std::cout << "Task f1A running...\n";
                                 sleep(5);
                                 std::cout << "Task f1A stopped\n";
                         }).name("f1A");
        tf::Task f1B = f1.emplace([]() { std::cout << "Task f1B\n"; }).name("f1B");

        // create taskflow f2 with one module task composed of f1
        tf::Task f2A = f2.emplace([]() { std::cout << "Task f2A\n"; }).name("f2A");
        tf::Task f2B = f2.emplace([]() { std::cout << "Task f2B\n"; }).name("f2B");
        tf::Task f2C = f2.emplace([]() { std::cout << "Task f2C\n"; }).name("f2C");

        tf::Task f1_module_task = f2.composed_of(f1).name("module");

        f1_module_task.succeed(f2A, f2B).precede(f2C);

        executor.run(f2).wait();

        return 0;
}