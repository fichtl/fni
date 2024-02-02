#include <iostream>
#include <stdio.h>

#include "taskflow/taskflow.hpp"

int main(int argc, char** argv)
{
        tf::Executor executor;

        tf::Taskflow taskflow;

        tf::Task A = taskflow.emplace([]() { printf("A\n"); });
        tf::Task B = taskflow.emplace([]() { printf("B\n"); });
        tf::Task C = taskflow.emplace([]() { printf("C\n"); });
        tf::Task D = taskflow.emplace([]() { printf("D\n"); });
        A.precede(B, C);
        D.succeed(B, C);

        // run once
        printf("- run_once starts -\n");
        tf::Future<void> run_once = executor.run(taskflow);
        run_once.wait();
        // above two lines can be merged: executor.run(taskflow).wait();
        printf("- run_once stops -\n");

        // run the taskflow four times
        printf("- run_n with 4 starts -\n");
        executor.run_n(taskflow, 4);
        executor.wait_for_all();
        // similarly, above two lines can be merged: xxxxxx.wait();
        printf("- run_n with 4 stops -\n");

        // runs the taskflow five times
        printf("- run_until with counter 5 starts -\n");
        executor.run_until(taskflow, [counter = 6]() mutable { return --counter == 0; });
        executor.wait_for_all();
        // similarly, above two lines can be merged: xxxxxx.wait();
        printf("- run_until with counter 5 stops -\n");

        return 0;
}
