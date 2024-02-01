#include <iostream>
#include <unistd.h>

#include "taskflow/taskflow.hpp"

int main(int argc, char** argv)
{
        tf::Executor executor;

        tf::Taskflow f1, f2;

        f1.name("F1");
        tf::Task f1A = f1.emplace([]() {
                                 std::cout << "Task f1A running...\n";
                                 sleep(5);
                                 std::cout << "Task f1A stopped\n";
                         }).name("f1A");
        tf::Task f1B = f1.emplace([]() { std::cout << "Task f1B\n"; }).name("f1B");
        tf::Task f1C = f1.emplace([]() { std::cout << "Task f1C\n"; }).name("f1C");
        f1A.precede(f1C);
        f1B.precede(f1C);

        f2.name("F2");
        tf::Task f2A = f2.emplace([]() { std::cout << "Task f2A\n"; }).name("f2A");
        tf::Task f2B = f2.emplace([]() { std::cout << "Task f2B\n"; }).name("f2B");
        tf::Task f2C = f2.emplace([]() { std::cout << "Task f2C\n"; }).name("f2C");
        tf::Task f2D = f2.emplace([]() { std::cout << "Task f2D\n"; }).name("f2D");
        f2A.precede(f2C);
        f2B.precede(f2C);

        tf::Task f1_module_task = f2.composed_of(f1).name("module");
        f1_module_task.succeed(f2C).precede(f2D);

        f2.dump(std::cout);

        executor.run(f2).wait();

        return 0;
}
