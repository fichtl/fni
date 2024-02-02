#include <string>

#include "dni/framework/dni.pb.h"

namespace dni {

        namespace utils {

                std::string CanonicalNodeName(const GraphConfig& config, int node_id)
                {
                        const auto& node_config = config.node(node_id);
                        std::string base_name = node_config.name().empty()
                                                    ? node_config.task()
                                                    : node_config.name();
                        int count = 0;
                        int seq = 0;
                        for (int i = 0; i < config.node_size(); ++i)
                        {
                                const auto& curr_node_config = config.node(i);
                                std::string curr_node_name =
                                    curr_node_config.name().empty()
                                        ? curr_node_config.task()
                                        : curr_node_config.name();
                                if (curr_node_name == base_name)
                                {
                                        ++count;
                                        if (i < node_id)
                                                ++seq;
                                }
                        }
                        if (count <= 1)
                                return base_name;
                        return base_name + "_" + std::to_string(seq);
                }

        }   // namespace utils

}   // namespace dni
