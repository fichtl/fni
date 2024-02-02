#include "dni/framework/node_config.h"

#include <memory>
#include <mutex>

#include "dni/framework/dtype.h"
#include "dni/framework/dni.pb.h"
#include "dni/framework/utils/tags.h"
#include "fmt/format.h"
#include "spdlog/spdlog.h"

namespace dni {

        int NodeConfig::Initialize(const GraphConfig::Node& node)
        {
                std::shared_ptr<utils::TagMap> input_tags =
                    utils::NewTagMap(node.input_stream());
                std::shared_ptr<utils::TagMap> output_tags =
                    utils::NewTagMap(node.output_stream());
                std::shared_ptr<utils::TagMap> input_side_data_tags =
                    utils::NewTagMap(node.input_side_data());
                std::shared_ptr<utils::TagMap> output_side_data_tags =
                    utils::NewTagMap(node.output_side_data());

                inputs_ = std::make_unique<DtypeSet>(std::move(input_tags));
                outputs_ = std::make_unique<DtypeSet>(std::move(output_tags));
                input_side_data_ =
                    std::make_unique<DtypeSet>(std::move(input_side_data_tags));
                output_side_data_ =
                    std::make_unique<DtypeSet>(std::move(output_side_data_tags));

                proto_ = &node;

                return 0;
        }

}   // namespace dni
