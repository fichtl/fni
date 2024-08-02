
#include <iostream>
#include <string>
#include <vector>

#include "spdlog/spdlog.h"

std::vector<std::string> label_relation_dict = {
    "BROWSING", "CHAT", "DDoS-hping3", "DDoS-pktgen", "DDoS-trafgen",
    "FTP",      "MAIL", "Streaming",   "VOIP",        "p2p"};

namespace dni {
std::string get_label_from_infer_ret(
    const std::vector<std::vector<std::vector<float_t>>>& infer_ret,
    int& label_relation_index)
{
        // only one output in this onnx model
        auto output_data = infer_ret[0];
        std::vector<int> model_output;
        for (auto&& od : output_data)
        {
                int maxPos = std::max_element(od.begin(), od.end()) - od.begin();
                model_output.push_back(maxPos);

                std::cout << maxPos << "  ";
        }
        std::cout << "model_output" << std::endl;

        std::vector<int> model_output_count;
        model_output_count.resize(label_relation_dict.size());
        for (auto&& maxPos : model_output)
        {
                model_output_count[maxPos]++;
        }
        for (auto&& i : model_output_count)
        {
                std::cout << i << "  ";
        }
        std::cout << "model_output_count" << std::endl;

        int predicted_class_id =
            std::max_element(model_output_count.begin(), model_output_count.end()) -
            model_output_count.begin();

        label_relation_index = predicted_class_id;

        SPDLOG_DEBUG("label_relation_index: {}", label_relation_index);

        auto predicted_class = label_relation_dict[predicted_class_id];

        return predicted_class;
}
}   // namespace dni