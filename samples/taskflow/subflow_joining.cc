#include <iostream>
#include <string>

#include "taskflow/taskflow.hpp"

int spawn(int n, tf::Subflow& sub)
{
        if (n < 2)
                return n;

        int res1, res2;
        sub.emplace([&res1, n](tf::Subflow& sub) { res1 = spawn(n - 1, sub); })
            .name(std::to_string(n - 1));
        sub.emplace([&res2, n](tf::Subflow& sub) { res2 = spawn(n - 2, sub); })
            .name(std::to_string(n - 2));
        sub.join();

        return res1 + res2;
}

int main(int argc, char** argv)
{
        tf::Taskflow taskflow;
        tf::Executor executor;

        int res;
        int n = 5;

        taskflow.emplace([&res, n](tf::Subflow& sub) { res = spawn(n, sub); })
            .name(std::to_string(n));

        executor.run(taskflow).wait();

        taskflow.dump(std::cout);

        std::cout << res << std::endl;
}
