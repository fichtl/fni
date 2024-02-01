#include <iostream>

#include "taskflow/taskflow.hpp"

struct CustomGraph {
        tf::Graph graph_;
        CustomGraph()
        {
                tf::FlowBuilder builder(graph_);
                builder.emplace([]() { std::cout << "a task" << std::endl; });
        }

        tf::Graph& graph() { return graph_; }
};

int main(int argc, char** argv)
{
        tf::Taskflow tf;
        tf::Executor executor;

        CustomGraph g;

        tf.composed_of(g);

        executor.run(tf).wait();

        return 0;
}
