#include <string>

#include "taskflow/taskflow.hpp"

class TaskBase {
public:
        TaskBase();
        virtual ~TaskBase();

        virtual int Open() = 0;
        virtual int Process() = 0;
        virtual int Close() = 0;

        std::string name;
};
TaskBase::TaskBase() {}
TaskBase::~TaskBase() {}

class Node {
public:
        Node(TaskBase* t) { task_ = t; }
        ~Node() { delete task_; }

        int Open() { return task_->Open(); }

        int Run()
        {
                // handle input, do calculation, handle output, exit
                int ret = task_->Process();
                return ret;
        }

        int Close() { return task_->Close(); }

        std::string Name() { return task_->name; }

        const TaskBase* task() { return task_; }

private:
        TaskBase* task_;
};

class Graph {
public:
        Graph(std::vector<Node*> tasks): nodes_(tasks) {}

        int Open()
        {
                for (auto node : nodes_)
                {
                        if (node->Open())
                                return -1;
                }
                return 0;
        }

        int Compose()
        {
                tf::Task last =
                    taskflow.emplace([&]() { nodes_[0]->Run(); }).name(nodes_[0]->Name());
                for (int i = 1; i < nodes_.size(); ++i)
                {
                        tf::Task t = taskflow.emplace([&, i]() { nodes_[i]->Run(); })
                                         .name(nodes_[i]->Name());
                        t.succeed(last);
                        last = t;
                }

                taskflow.dump(std::cout);
        }

        int Run(int n)
        {
                future = executor.run_n(taskflow, n);
                return 0;
        }
        int Run() { return Run(1); }

        int Wait() { future.wait(); }

        int Close()
        {
                for (auto node : nodes_) node->Close();
                return 0;
        }

        tf::Graph& graph() { return taskflow.graph(); }

private:
        std::vector<Node*> nodes_;

        tf::Executor executor;
        tf::Taskflow taskflow;
        tf::Future<void> future;
};
