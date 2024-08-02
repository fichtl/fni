#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace dni {
void get_mfr_bytes(
    std::vector<std::string>& mfr_bytes_list, unsigned char* pktsdata, uint32_t pkts_cnt,
    int batch_size = 64);

void get_model_input(std::vector<std::string>& mfr_bytes_list, float* model_np_list);

}   // namespace dni