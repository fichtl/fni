#include "taskflow/algorithm/pipeline.hpp"

#include "taskflow/taskflow.hpp"

int main(int argc, char** argv)
{
        tf::Taskflow taskflow;
        tf::Executor executor;

        const size_t num_lines = 4;
        const size_t num_pipes = 3;

        // create a custom data buffer
        std::array<std::array<int, num_pipes>, num_lines> buffer;

        // create a pipeline graph of four concurrent lines and three serial pipes
        tf::Pipeline pipeline(
            num_lines,
            // first pipe must define a serial direction
            tf::Pipe{
                tf::PipeType::SERIAL,
                [&buffer](tf::Pipeflow& pf) {
                        // generate only 5 scheduling tokens
                        if (pf.token() == 5)
                        {
                                pf.stop();
                        }
                        // save the token id into the buffer
                        else
                        {
                                buffer[pf.line()][pf.pipe()] = pf.token();
                        }
                }},
            tf::Pipe{
                tf::PipeType::SERIAL,
                [&buffer](tf::Pipeflow& pf) {
                        // propagate the previous result to this pipe by adding one
                        buffer[pf.line()][pf.pipe()] =
                            buffer[pf.line()][pf.pipe() - 1] + 1;
                }},
            tf::Pipe{tf::PipeType::SERIAL, [&buffer](tf::Pipeflow& pf) {
                             // propagate the previous result to this pipe by adding one
                             buffer[pf.line()][pf.pipe()] =
                                 buffer[pf.line()][pf.pipe() - 1] + 1;
                     }});

        // build the pipeline graph using composition
        tf::Task init =
            taskflow.emplace([]() { std::cout << "ready\n"; }).name("starting pipeline");
        tf::Task task = taskflow.composed_of(pipeline).name("pipeline");
        tf::Task stop =
            taskflow.emplace([]() { std::cout << "stopped\n"; }).name("pipeline stopped");

        // create task dependency
        init.precede(task);
        task.precede(stop);

        // run the pipeline
        executor.run(taskflow).wait();

        for (size_t i = 0; i < num_lines; ++i)
        {
                for (size_t j = 0; j < num_pipes; ++j)
                {
                        std::cout << buffer[i][j] << " ";
                }
                std::cout << std::endl;
        }
}
