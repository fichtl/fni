#include <memory>
#include <string>

#include "pipeline.h"

class NopTask: public TaskBase {
public:
        NopTask() { ++num_constructed_; }
        ~NopTask() override { ++num_destroyed_; }

        int Open() override { return 0; }

        int Process() override
        {
                ++num_called_;
                std::cout << name << ": processing data" << std::endl;
                return 0;
        }

        int Close() override { return 0; }

        int num_called_;

        static int num_constructed_;
        static int num_destroyed_;
};

int NopTask::num_constructed_ = 0;
int NopTask::num_destroyed_ = 0;

int main(int argc, char** argv)
{
        std::vector<Node*> nodes(20);

        NopTask* t;
        for (int i = 0; i < nodes.size(); ++i)
        {
                t = new NopTask();
                t->name = "task " + std::to_string(i);
                nodes[i] = new Node(t);
        }
        std::cout << NopTask::num_constructed_ << std::endl;

        Graph graph(nodes);
        graph.Compose();
        graph.Run(3);
        graph.Wait();

        for (auto node : nodes)
        {
                std::cout << "num called: " << ((NopTask*) node->task())->num_called_
                          << std::endl;
        }

        for (auto node : nodes)
        {
                delete node;
        }
        std::cout << NopTask::num_destroyed_ << std::endl;

        return 0;
}
