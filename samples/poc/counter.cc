#include <string>
#include <unistd.h>

#include "pipeline.h"

using Datum = int;

class CountTask: public TaskBase {
public:
        CountTask()
        {
                ++num_constructed_;
                SetupInput();
        }
        ~CountTask() override { ++num_destroyed_; }

        void SetupInput() { input_handler_ = new Datum(); };
        void SetupOutput(Datum* input_handler) { output_handler_ = input_handler; }
        int Open() override { return 0; }

        Datum GetInput() { return *input_handler_; }
        void Propagate() { *output_handler_ = data_; }
        int Process() override
        {
                ++num_called_;

                // Check if input is ready
                while (!GetInput()) continue;

                // Handle inputs
                data_ = *input_handler_ + 1;

                // Clear processed input
                *input_handler_ = 0;

                // Check if output exists
                if (!output_handler_)
                        return 0;

                Propagate();

                return 0;
        }

        int Close() override { return 0; }

        int num_called_;

        Datum* input_handler_;
        Datum data_;
        Datum* output_handler_;

        static int num_constructed_;
        static int num_destroyed_;
};

int CountTask::num_constructed_ = 0;
int CountTask::num_destroyed_ = 0;

int main(int argc, char** argv)
{
        std::vector<Node*> nodes(20);

        CountTask *t, *last;
        for (int i = 0; i < nodes.size(); ++i)
        {
                t = new CountTask();
                t->name = "task " + std::to_string(i);
                nodes[i] = new Node(t);
                if (i == 0)
                {
                        last = t;
                        continue;
                }
                last->SetupOutput(t->input_handler_);
                last = t;
        }
        std::cout << CountTask::num_constructed_ << std::endl;

        for (int i = 0; i < nodes.size(); ++i)
        {
                t = (CountTask*) nodes[i]->task();
                std::cout << "input: " << t->GetInput()
                          << ", input_handler: " << t->input_handler_
                          << ", output_handler: " << t->output_handler_ << std::endl;
        }

        Graph graph(nodes);
        graph.Compose();

        tf::Executor executor;
        tf::Taskflow taskflow;

        t = (CountTask*) nodes[0]->task();
        auto a = taskflow.emplace([&]() { *t->input_handler_ = 1; });
        auto b = taskflow.composed_of(graph);
        a.precede(b);

        executor.run_n(taskflow, 100000).wait();

        for (int i = 0; i < nodes.size(); ++i)
        {
                t = (CountTask*) nodes[i]->task();
                std::cout << "data: " << t->data_ << std::endl;
        }

        for (auto node : nodes)
        {
                delete node;
        }
        std::cout << CountTask::num_destroyed_ << std::endl;

        return 0;
}
