#include <tuple>

#include "taskflow/taskflow.hpp"

int main(int argc, char** argv)
{
        tf::Taskflow taskflow;
        tf::Executor executor;

        tf::Task A, B, C, D;

        std::tie(A, B, C, D) = taskflow.emplace(
            []() { return 0; },
            [&C](tf::Runtime& rt) {
                    std::cout << "B\n";
                    rt.schedule(C);
            },
            []() { std::cout << "C\n"; }, []() { std::cout << "D\n"; });

        A.precede(B, C, D);

        executor.run(taskflow).wait();
}
