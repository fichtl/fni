#pragma once

#include "dni/framework/dni.pb.h"

namespace dni {

        namespace utils {

                std::string CanonicalNodeName(const GraphConfig& config, int node_id);

        }

}   // namespace dni
