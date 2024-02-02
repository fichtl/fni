#include <iostream>

#include "taskflow/taskflow.hpp"

int main(int argc, char** argv)
{
        tf::Taskflow taskflow;
        tf::Executor executor;

        int val = 0;

        auto [init, body, cond, done] = taskflow.emplace(
            [&]() { std::cout << "init.\n"; },
            [&]() {
                    std::cout << "val: " << val << std::endl;
                    ++val;
            },
            [&]() { return val < 5 ? 0 : 1; }, [&]() { std::cout << "done.\n"; });

        init.precede(body);
        body.precede(cond);
        cond.precede(body, done);

        executor.run(taskflow).wait();

        return 0;
}
