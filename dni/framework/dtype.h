#pragma once

#include <memory>

#include "dni/framework/utils/tags.h"

namespace dni {

        // TODO: type system is not urgent.
        class Dtype {};

        // TODO: type system is not urgent.
        class DtypeSet {
        public:
                DtypeSet(std::shared_ptr<utils::TagMap> tag_map): tag_map_(tag_map){};

                int FindByTagIndex(const std::string& tag, int index)
                {
                        return tag_map_->FindByTagIndex(tag, index);
                }

                int size() const { return tag_map_->NStreams(); }

                const std::shared_ptr<utils::TagMap>& TagMap() const { return tag_map_; }

        private:
                std::shared_ptr<utils::TagMap> tag_map_;
        };

}   // namespace dni
