#include <chrono>
#include <thread>
#include <vector>

#include "dni/framework/framework.h"
#include "dni/tasks/onnx/onnx.h"
#include "spdlog/spdlog.h"

/*

// refer to onnxruntime/test/shared_lib/test_nontensor_types.cc:
// CreateGetVectorOfMapsInt64Float test


[
[6.0081 1.9779 -6.9387 ]
[5.27597 1.81978 -6.18197 ]
]


*/

void userPrepareInputData(std::vector<std::vector<float>*>& input_data)
{
        std::vector<float>* input_tensor2 = new std::vector<float>();

        input_tensor2->push_back((float) 5.1);
        input_tensor2->push_back((float) 3.5);
        input_tensor2->push_back((float) 1.4);
        input_tensor2->push_back((float) 0.2);

        input_tensor2->push_back((float) 4.9);
        input_tensor2->push_back((float) 3.0);
        input_tensor2->push_back((float) 1.4);
        input_tensor2->push_back((float) 0.2);

        input_data.push_back(input_tensor2);
}

void inject_after(dni::Graph* g, int after, int interval)
{
        std::this_thread::sleep_for(std::chrono::milliseconds(after));

        std::vector<std::vector<float>*> input_data;
        userPrepareInputData(input_data);   // user call

        g->AddDatumToInputStream("data", dni::Datum(input_data));

        std::this_thread::sleep_for(std::chrono::milliseconds(interval));
}

int main()
{
        spdlog::set_level(spdlog::level::trace);

        const std::string& proto = R"pb(
                type: "Onnx-BPNet-Iris"

                input_stream: "GIn_Data:0:data"
                output_stream: "GOut_Ret:0:ret"

                node {
                  name: "A"
                  task: "OnnxTwoDimesionTask"
                  input_stream: "GIn_Data:0:data"
                  output_stream: "GOut_Ret:0:ret"

                  options {
                    [type.asnapis.io/dni.OnnxTaskOptions] {
                      model_path: "samples/dni/tasks/onnx/testdata/bpnet_iris.onnx"
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
        inject_after(g, 0, 1);

        g->RunOnce();

        g->Wait();

        auto ret = g->GetResult<std::vector<std::vector<std::vector<float_t>>>>(out);

        spdlog::info("Gout {} result is: {}", out, ret);

        g->Finish();

        spdlog::info("main over");

        return 0;
}
