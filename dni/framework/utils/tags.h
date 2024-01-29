#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace dni {

        namespace utils {

                struct TagData {
                        TagData(int id, int count): id(id), count(count) {}
                        int id;
                        int count;
                };

                class TagMap {
                public:
                        std::shared_ptr<TagMap> Create(std::string& name)
                        {
                                std::shared_ptr<TagMap> ret(new TagMap());
                                return std::move(ret);
                        }

                        const std::unordered_map<std::string, TagData>& Map() const
                        {
                                return data_;
                        }

                private:
                        TagMap();

                        int Initialize(std::string& names);

                        std::unordered_map<std::string, TagData> data_;

                        std::vector<std::string> names_;
                };

        }   // namespace utils

}   // namespace dni
