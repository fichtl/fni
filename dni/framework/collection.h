#pragma once

#include "dni/framework/utils/tags.h"
#include "spdlog/spdlog.h"

namespace dni {

        template <typename T>
        class Collection {
        public:
                using value_t = T;
                using stored_t = T;
                using pointer = value_t*;
                using reference = value_t&;
                using const_reference = const value_t&;
                using iterator = pointer;
                using const_iterator = const pointer;

                Collection() = delete;
                Collection(const Collection&) = delete;
                Collection operator=(const Collection&) = delete;

                explicit Collection(std::shared_ptr<utils::TagMap> tag_map)
                    : tag_map_(std::move(tag_map))
                {
                        if (tag_map_->NStreams() != 0)
                        {
                                data_ =
                                    std::make_unique<stored_t[]>(tag_map_->NStreams());
                        }
                }

                int Slot(const std::string_view tag, int index)
                {
                        return tag_map_->FindByTagIndex(tag, index);
                }

                value_t& Get(const std::string_view tag, int index)
                {
                        int id = Slot(tag, index);
                        if (id < 0)
                        {
                                SPDLOG_CRITICAL("unexpected tag:index {}:{}", tag, index);
                                std::abort();
                        }
                        return begin()[id];
                }
                const value_t& Get(const std::string_view tag, int index) const
                {
                        int id = Slot(tag, index);
                        if (id < 0)
                        {
                                SPDLOG_CRITICAL("unexpected tag:index {}:{}", tag, index);
                                std::abort();
                        }
                        return begin()[id];
                }

                value_t& Tag(const std::string_view tag) { return Get(tag, 0); }
                const value_t& Tag(const std::string_view tag) const
                {
                        return Get(tag, 0);
                }

                value_t& Index(int index) { return Get("", index); }
                const value_t& Index(int index) const { return Get("", index); }

                int size() const { return tag_map_->NStreams(); }

                reference operator[](int n) { return begin()[n]; }
                const_reference operator[](int n) const { return begin()[n]; }
                reference at(int n) { return begin()[n]; }
                const_reference at(int n) const { return begin()[n]; }

                iterator begin() { return iterator(data_.get()); }
                iterator end() { return iterator(data_.get() + tag_map_->NStreams()); }
                const_iterator begin() const { return const_iterator(data_.get()); }
                const_iterator end() const
                {
                        return const_iterator(data_.get() + tag_map_->NStreams());
                }

                const std::shared_ptr<utils::TagMap>& TagMap() const { return tag_map_; }

        private:
                std::shared_ptr<utils::TagMap> tag_map_;

                std::unique_ptr<stored_t[]> data_;
        };

}   // namespace dni
