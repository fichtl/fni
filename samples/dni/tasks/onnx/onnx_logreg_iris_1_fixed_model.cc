#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

#include "dni/framework/framework.h"
#include "dni/tasks/onnx/onnx.h"
#include "onnx_defines.h"
#include "spdlog/spdlog.h"

/*
[[0 ]],              output_labels_total: 1

value:
0.968525		0.0314749		1.00007e-07


[[2 ]],              output_labels_total: 1

value:
1.37622e-05		0.026101		0.973885

*/

void userPrepareInputData1(std::vector<std::vector<float>*>& input_data)
{
        std::vector<float>* input_tensor = new std::vector<float>();
        input_tensor->push_back((float) 5.1);
        input_tensor->push_back((float) 3.4);
        input_tensor->push_back((float) 1.5);
        input_tensor->push_back((float) 0.2);
        input_data.push_back(input_tensor);
}

void inject_after1(dni::Graph* g, int after, int interval)
{
        std::this_thread::sleep_for(std::chrono::milliseconds(after));

        std::vector<std::vector<float>*> input_data;
        userPrepareInputData1(input_data);   // user call

        g->AddDatumToInputStream("data", dni::Datum(input_data));

        std::this_thread::sleep_for(std::chrono::milliseconds(interval));
}

void userPrepareInputData2(std::vector<std::vector<float>*>& input_data)
{
        std::vector<float>* input_tensor = new std::vector<float>();
        input_tensor->push_back((float) 6.7);
        input_tensor->push_back((float) 3.2);
        input_tensor->push_back((float) 5.7);
        input_tensor->push_back((float) 2.3);
        input_data.push_back(input_tensor);
}

void inject_after2(dni::Graph* g, int after, int interval)
{
        std::this_thread::sleep_for(std::chrono::milliseconds(after));

        std::vector<std::vector<float>*> input_data;
        userPrepareInputData2(input_data);   // user call

        g->AddDatumToInputStream("data", dni::Datum(input_data));

        std::this_thread::sleep_for(std::chrono::milliseconds(interval));
}

int main()
{
        spdlog::set_level(spdlog::level::trace);

        const std::string& proto = R"pb(
                type: "Onnx-LogReg-Iris-Fxied-Model"

                input_stream: "GIn_Data:0:data"
                output_stream: "GOut_Ret:0:ret"

                node {
                  name: "A"
                  task: "OnnxLogRegTask"
                  input_stream: "GIn_Data:0:data"
                  output_stream: "GOut_Ret:0:ret"

                  options {
                    [type.asnapis.io/dni.OnnxTaskOptions] {
                      model_path: "samples/dni/tasks/onnx/testdata/logreg_iris_fixed_model.onnx"
                    }
                  }
                }
        )pb";

        auto gc = dni::ParseStringToGraphConfig(proto);
        if (!gc)
        {
                spdlog::error("invalid pbtxt config: {}", proto);
                return -1;
        }
        spdlog::debug(
            "GraphConfig: {}, node size: {}", gc.value().type(), gc.value().node_size());

        dni::Graph* g = new dni::Graph(gc.value());

        std::string out = "ret";

        spdlog::debug("Create ObserveOutputStream: {}", out);
        g->ObserveOutputStream(out);

        g->PrepareForRun();
        inject_after1(g, 0, 1);

        g->RunOnce();

        g->Wait();

        auto ret = g->GetResult<dni::LogRegInferenceRet>(out);

        spdlog::info("Gout {} result size is: {}", out, ret.output_labels.size());

        for (int i = 0; i < ret.output_labels.size(); i++)
        {
                spdlog::info("output_label is: {}", ret.output_labels[i]);
        }
        for (int i = 0; i < ret.output_probabilities.size(); i++)
        {
                spdlog::info(
                    "output_probabilities-{} is: {}", i, ret.output_probabilities[i]);
        }

        // test2
        inject_after2(g, 0, 1);

        g->RunOnce();

        g->Wait();

        auto ret2 = g->GetResult<dni::LogRegInferenceRet>(out);

        spdlog::info("Gout {} result size is: {}", out, ret2.output_labels.size());

        for (int i = 0; i < ret2.output_labels.size(); i++)
        {
                spdlog::info("output_label is: {}", ret2.output_labels[i]);
        }
        for (int i = 0; i < ret2.output_probabilities.size(); i++)
        {
                spdlog::info(
                    "output_probabilities-{} is: {}", i, ret2.output_probabilities[i]);
        }

        g->ClearResult();

        g->Finish();

        spdlog::info("main over");

        return 0;
}
