#pragma once

#include <memory>
#include <vector>

#include "dni/framework/datum.h"
#include "dni/framework/utils/tags.h"

namespace dni {

        // TODO: type system is not urgent.
        class Dtype {
        public:
                // not implemented yet.
                int Validate(const Datum&) const { return 0; }
        };

        // TODO: type system is not urgent.
        class DtypeSet {
        public:
                DtypeSet(std::shared_ptr<utils::TagMap> tag_map)
                    : tag_map_(std::move(tag_map))
                {
                        if (tag_map_->NStreams() != 0)
                        {
                                data_ = std::vector<Dtype>(tag_map_->NStreams());
                        }
                };

                int Slot(const std::string_view tag, int index)
                {
                        return tag_map_->FindByTagIndex(tag, index);
                }

                Dtype& at(int i) { return data_[i]; }
                const Dtype& at(int i) const { return data_[i]; }

                int size() const { return tag_map_->NStreams(); }

                const std::shared_ptr<utils::TagMap>& TagMap() const { return tag_map_; }

        private:
                std::shared_ptr<utils::TagMap> tag_map_;
                std::vector<Dtype> data_;
        };

}   // namespace dni
