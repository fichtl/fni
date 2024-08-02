#pragma once

#include <string>
#include <vector>

namespace dni {

std::string get_label_from_infer_ret(
    const std::vector<std::vector<std::vector<float_t>>>& infer_ret,
    int& label_relation_index);

}   // namespace dni