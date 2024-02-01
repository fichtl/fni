#include <string>
#include <unistd.h>

#include "taskflow/taskflow.hpp"

void trivial_task(const std::string& name) { std::cout << name << " done." << std::endl; }

void nontrivial_task(const std::string& name)
{
        sleep(1);
        std::cout << name << " done." << std::endl;
}

int main(int argc, char** argv)
{
        tf::Taskflow taskflow;
        tf::Executor executor;

        tf::Task A = taskflow.emplace([]() { nontrivial_task("A"); }).name("A");
        tf::Task C = taskflow.emplace([]() {}).name("C");
        tf::Task D = taskflow.emplace([]() {}).name("D");

        int acc = 0;

        tf::Task B = taskflow
                         .emplace([&](tf::Subflow& subflow) {
                                 tf::Task B1 = subflow
                                                   .emplace([&]() {
                                                           ++acc;
                                                           trivial_task("B1");
                                                   })
                                                   .name("B1");
                                 tf::Task B2 = subflow
                                                   .emplace([&]() {
                                                           ++acc;
                                                           trivial_task("B2");
                                                   })
                                                   .name("B2");
                                 tf::Task B3 = subflow
                                                   .emplace([&]() {
                                                           ++acc;
                                                           trivial_task("B3");
                                                   })
                                                   .name("B3");
                                 B1.precede(B3);
                                 B2.precede(B3);
                                 subflow.detach();
                         })
                         .name("B");

        A.precede(B);
        A.precede(C);
        B.precede(D);
        C.precede(D);

        executor.run_n(taskflow, 3).wait();

        std::cout << "acc: " << acc << std::endl;

        taskflow.dump(std::cout);

        std::cout << "num of tasks: " << taskflow.num_tasks() << std::endl;
        taskflow.for_each_task([](tf::Task&& t) { std::cout << t.name() << std::endl; });
}
