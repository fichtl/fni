#include <iostream>
#include <taskflow/algorithm/for_each.hpp>
#include <taskflow/algorithm/reduce.hpp>
#include <taskflow/algorithm/sort.hpp>
#include <taskflow/taskflow.hpp>
#include <vector>

int main(int argc, char** argv)
{
        tf::Executor executor;
        tf::Taskflow taskflow;

        std::vector<int> vec = std::vector<int>{0, 1, 2, 3, 4, 5};
        int acc = 0;
        auto first = vec.begin(), last = vec.end();

        tf::Task task1 = taskflow.for_each(first, last, [](auto& i) { i = 100; });

        tf::Task task2 =
            taskflow.reduce(first, last, acc, [](auto a, auto b) { return a + b; });

        tf::Task task3 = taskflow.sort(first, last, [](auto a, auto b) { return a > b; });

        tf::Task task4 = taskflow.placeholder().name("traversal task after sorting");
        task4.data(&vec).work([task4]() {
                auto data = *static_cast<std::vector<int>*>(task4.data());
                std::cout << "value when running " << task4.name() << ": ";
                for (auto i = data.begin(); i != data.end(); i++)
                {
                        std::cout << *i << " ";
                }
                std::cout << std::endl;
        });

        task4.succeed(task3);
        task4.precede(task1);
        task2.precede(task1);

        executor.run(taskflow).wait();

        std::cout << "accumulated value for reducing: " << acc << std::endl;

        std::cout << "vector after entire taskflow: ";
        for (auto i = first; i != last; i++)
        {
                std::cout << *i << " ";
        }

        return 0;
}