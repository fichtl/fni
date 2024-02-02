#include <iostream>

#include "taskflow/taskflow.hpp"

int main(int argc, char** argv)
{
        tf::Executor executor;
        tf::Taskflow taskflow;

        tf::Task init = taskflow.emplace([]() { std::cout << "init\n"; }).name("init");
        tf::Task stop = taskflow.emplace([]() { std::cout << "stop\n"; }).name("stop");

        tf::Task cond = taskflow
                            .emplace([]() {
                                    std::cout << "cond\n";
                                    return std::rand() % 2;
                            })
                            .name("cond");

        init.precede(cond);
        cond.precede(cond, stop);

        executor.run(taskflow).wait();

        return 0;
}
