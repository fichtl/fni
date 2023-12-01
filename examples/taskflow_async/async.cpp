#include <iostream>
#include <stdio.h>

#include "taskflow/taskflow.hpp"

// Note: Executor-created async tasks do not belong to *any* taskflows. The lifetime of
// an async task is managed automatically by the executor that creates the task.
void async_tasks_from_executor()
{
        tf::Executor executor;

        // 1. create asynchronous tasks directly from an executor
        std::future<int> future = executor.async("async task with return value", []() {
                std::cout << "async task returns 1\n";
                return 1;
        });
        assert(future.get() == 1);

        // 2. create asynchronous silent tasks directly from an executor
        executor.silent_async("async task without return value", []() {
                std::cout << "async task does not return\n";
        });

        // 3. create asynchronous tasks with dynamic dependencies
        tf::AsyncTask A =
            executor.silent_dependent_async("task A", []() { printf("A\n"); });
        tf::AsyncTask B = executor.silent_dependent_async([]() { printf("B\n"); }, A);
        tf::AsyncTask C = executor.silent_dependent_async([]() { printf("C\n"); }, A);
        tf::AsyncTask D = executor.silent_dependent_async([]() { printf("D\n"); }, B, C);

        // 4. create asynchronous tasks from an external thread or a worker thread
        tf::Taskflow taskflow;
        tf::Task E = taskflow.emplace([&]() {
                executor.async([&]() {
                        printf("E0\n");
                        executor.async([&]() { printf("E1\n"); });
                });
        });
        executor.run(taskflow);

        executor.wait_for_all();
}

void async_tasks_from_subflow()
{
        tf::Taskflow taskflow;
        tf::Executor executor;

        std::atomic<int> counter{0};

        taskflow.emplace([&](tf::Subflow& sf) {
                std::vector<std::future<void>> futures;
                for (int i = 0; i < 100; i++)
                {
                        futures.emplace_back(sf.async([&]() { ++counter; }));
                }
                // Subflow should be joinable, or it may result in undefined behavior.
                sf.join();
                assert(counter == 100);
                std::cout << "final value of counter is: " << counter << std::endl;
        });

        executor.run(taskflow).wait();
}

void async_tasks_from_runtime()
{
        tf::Taskflow taskflow;
        tf::Executor executor;

        std::atomic<int> counter{0};

        taskflow.emplace([&](tf::Runtime& rt) {
                for (int i = 0; i < 100; i++)
                {
                        rt.silent_async([&]() { ++counter; });
                }
                rt.corun_all();
                assert(counter == 100);
                std::cout << "value of counter for the first round is: " << counter
                          << std::endl;

                for (int i = 0; i < 100; i++)
                {
                        rt.silent_async([&]() { ++counter; });
                }
                rt.corun_all();
                assert(counter == 200);
                std::cout << "final value of counter is: " << counter << std::endl;
        });

        executor.run(taskflow).wait();
}

int main(int argc, char** argv)
{
        std::cout << "Launch asynchronous tasks from an Executor:\n";
        async_tasks_from_executor();
        std::cout << "---\n";

        std::cout << "Launch asynchronous tasks from a Subflow:\n";
        async_tasks_from_subflow();
        std::cout << "---\n";

        std::cout << "Launch asynchronous tasks from a Runtime:\n";
        async_tasks_from_runtime();
        std::cout << "---\n";

        return 0;
}