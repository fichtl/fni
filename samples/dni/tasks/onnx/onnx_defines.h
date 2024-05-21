#pragma once

#include <unordered_map>
#include <vector>

namespace dni {
struct LogRegInferenceRet {
        std::vector<std::vector<std::vector<int64_t>>> output_labels;
        std::vector<std::unordered_map<int64_t, float_t>> output_probabilities;
};
}   // namespace dni
