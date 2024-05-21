#include <chrono>
#include <thread>
#include <vector>

#include "dni/framework/framework.h"
#include "dni/tasks/onnx/onnx.h"
#include "spdlog/spdlog.h"

/*
same as the python code with numpy,

import numpy as np

x1 = np.array([[1, 2, 3, 4, 5], [6, 7, 8, 9, 10]])
x2 = np.array([[11, 12, 13], [14, 15, 16], [17, 18, 19], [20, 21, 22], [23, 24, 25]])
x3 = np.array([[211, 212, 213], [214, 215, 216]])
# （2，3）*（3，2）=（2，2）
print(np.add(np.dot(x1,x2), x3))


[[ 496  512  528]
 [ 924  965 1006]]


*/

/*
onnx model file can be made with code,

import onnx
from onnx import helper
from onnx import TensorProto
from onnx import version_converter

# input and output
a = helper.make_tensor_value_info('a', TensorProto.FLOAT, [2, 5])
x = helper.make_tensor_value_info('x', TensorProto.FLOAT, [5, 3])
b = helper.make_tensor_value_info('b', TensorProto.FLOAT, [2, 3])
output = helper.make_tensor_value_info('output', TensorProto.FLOAT, [2,3])
# MatMul, not 'dot' or '@'
mul = helper.make_node('MatMul', ['a', 'x'], ['c'])
# Add
add = helper.make_node('Add', ['c', 'b'], ['output'])
# graph and model
graph = helper.make_graph([mul, add], 'linear_func', [a, x, b], [output])
model = helper.make_model(graph)
# save model
onnx.checker.check_model(model)
# print(model)
onnx.save(model, 'model_2_5_x_5_3.onnx')

# need to convert to onnx version 19
model_path = "./model_2_5_x_5_3.onnx"
original_model = onnx.load(model_path)

# print(f"The model before conversion:\n{original_model}")

# A full list of supported adapters can be found here:
# https://github.com/onnx/onnx/blob/main/onnx/version_converter.py#L21
# Apply the version conversion on the original model
converted_model = version_converter.convert_version(original_model, 19)

# print(f"The model after conversion:\n{converted_model}")

onnx.save(converted_model, 'model_2_5_x_5_3_v19.onnx')

*/

void userPrepareInputData(std::vector<std::vector<float>*>& input_data)
{
        std::vector<float>* input_tensor_values1 = new std::vector<float>();
        input_tensor_values1->push_back((float) 1.00);
        input_tensor_values1->push_back((float) 2.00);
        input_tensor_values1->push_back((float) 3.00);
        input_tensor_values1->push_back((float) 4.00);
        input_tensor_values1->push_back((float) 5.00);
        input_tensor_values1->push_back((float) 6.00);
        input_tensor_values1->push_back((float) 7.00);
        input_tensor_values1->push_back((float) 8.00);
        input_tensor_values1->push_back((float) 9.00);
        input_tensor_values1->push_back((float) 10.00);

        input_data.push_back(input_tensor_values1);

        std::vector<float>* input_tensor_values2 = new std::vector<float>();
        input_tensor_values2->push_back((float) 11.00);
        input_tensor_values2->push_back((float) 12.00);
        input_tensor_values2->push_back((float) 13.00);
        input_tensor_values2->push_back((float) 14.00);
        input_tensor_values2->push_back((float) 15.00);
        input_tensor_values2->push_back((float) 16.00);
        input_tensor_values2->push_back((float) 17.00);
        input_tensor_values2->push_back((float) 18.00);
        input_tensor_values2->push_back((float) 19.00);
        input_tensor_values2->push_back((float) 20.00);
        input_tensor_values2->push_back((float) 21.00);
        input_tensor_values2->push_back((float) 22.00);
        input_tensor_values2->push_back((float) 23.00);
        input_tensor_values2->push_back((float) 24.00);
        input_tensor_values2->push_back((float) 25.00);
        input_data.push_back(input_tensor_values2);

        std::vector<float>* input_tensor_values3 = new std::vector<float>();
        input_tensor_values3->push_back((float) 211.00);
        input_tensor_values3->push_back((float) 212.00);
        input_tensor_values3->push_back((float) 213.00);
        input_tensor_values3->push_back((float) 214.00);
        input_tensor_values3->push_back((float) 215.00);
        input_tensor_values3->push_back((float) 216.00);
        input_data.push_back(input_tensor_values3);
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
                type: "Onnx-MatMul-Add"

                input_stream: "GIn_Data:0:data"
                output_stream: "GOut_Ret:0:ret"

                node {
                  name: "A"
                  task: "OnnxTwoDimesionTask"
                  input_stream: "GIn_Data:0:data"
                  output_stream: "GOut_Ret:0:ret"

                  options {
                    [type.asnapis.io/dni.OnnxTaskOptions] {
                      model_path: "samples/dni/tasks/onnx/testdata/model_2_5_x_5_3_v19.onnx"
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
