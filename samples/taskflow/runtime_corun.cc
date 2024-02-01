#include "taskflow/taskflow.hpp"

int main(int argc, char** argv)
{
        tf::Taskflow taskflow;
        tf::Executor executor(2);

        taskflow.emplace([](tf::Runtime& rt) {
                rt.corun([](tf::Subflow& sf) {
                        sf.emplace([]() { std::cout << "independent task 1\n"; });
                        sf.emplace([]() { std::cout << "independent task 2\n"; });
                });
        });
        executor.run_n(taskflow, 10000).wait();

        taskflow.clear();

        tf::Taskflow graph;
        graph.emplace(
            []() { std::cout << "independent task 1\n"; },
            []() { std::cout << "independent task 2\n"; });
        taskflow.emplace([&graph](tf::Runtime& rt) { rt.corun(graph); });
        executor.run_n(taskflow, 10000).wait();

        taskflow.clear();

        std::array<tf::Taskflow, 1000> others;
        std::atomic<size_t> counter{0};
        for (size_t n = 0; n < 1000; ++n)
        {
                for (size_t i = 0; i < 500; ++i)
                {
                        others[n].emplace([&]() { ++counter; });
                }
                taskflow.emplace([&tf = others[n]](tf::Runtime& rt) { rt.corun(tf); });
        }
        executor.run(taskflow).wait();
        std::cout << counter << std::endl;

        return 0;
}
