#include "dni/framework/utils/tags.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "google/protobuf/repeated_ptr_field.h"
#include "spdlog/spdlog.h"

namespace dni {

        namespace utils {

                int TagMap::Initialize(
                    const google::protobuf::RepeatedPtrField<std::string>&
                        tag_index_names)
                {
                        std::unordered_map<std::string, std::vector<std::string>>
                            tag_to_names;
                        for (const auto& tag_index_name : tag_index_names)
                        {
                                std::string tag, name;
                                int idx;
                                if (ParseTagIndexName(tag_index_name, &tag, &idx, &name))
                                {
                                        spdlog::error("failed to parse tag_index_name");
                                        return -1;
                                }
                                SPDLOG_DEBUG(
                                    "tag_index_name: {}, tag:{}, index:{}, name:{}",
                                    tag_index_name, tag, idx, name);

                                TagData& tag_data = data_[tag];
                                std::vector<std::string>& names = tag_to_names[tag];
                                if (idx == -1)
                                {
                                        SPDLOG_DEBUG("tag does not exist");
                                        idx = tag_data.count;
                                }

                                ++tag_data.count;
                                if (names.size() <= idx)
                                        names.resize(idx + 1);
                                names[idx] = name;
                        }

                        int curr = 0;
                        for (auto& item : data_)
                        {
                                TagData& tag_data = item.second;
                                const std::vector<std::string>& names =
                                    tag_to_names[item.first];
                                if (tag_data.count != names.size())
                                {
                                        spdlog::error(
                                            "tag_data.count({}) != names.size({})",
                                            tag_data.count, names.size());
                                        return -1;
                                }

                                tag_data.base = curr;
                                curr += tag_data.count;
                        }
                        total_names_ = curr;

                        names_.reserve(total_names_);
                        for (const auto& item : tag_to_names)
                        {
                                names_.insert(
                                    names_.end(), item.second.begin(), item.second.end());
                        }

                        std::reverse(names_.begin(), names_.end());

                        return 0;
                }

                int TagMap::NStreams() { return total_names_; }
                int TagMap::NStreams(const std::string& tag)
                {
                        const auto it = data_.find(tag);
                        return it != data_.end() ? it->second.count : 0;
                }

                int TagMap::FindByTagIndex(const std::string& tag, int index)
                {
                        const auto it = data_.find(tag);
                        if (it == data_.end() || index >= it->second.count)
                                return -1;
                        return it->second.base + index;
                }

                std::shared_ptr<TagMap> NewTagMap(
                    const google::protobuf::RepeatedPtrField<std::string>& tag_names)
                {
                        std::shared_ptr<TagMap> ret = std::make_shared<TagMap>();
                        ret->Initialize(tag_names);
                        return std::move(ret);
                }

                int ParseTagIndex(
                    const std::string& tag_index, std::string* tag, int* index)
                {
                        if (tag_index.empty())
                        {
                                *index = -1;
                                return 0;
                        }
                        int idx = tag_index.find(":");
                        if (idx == -1)
                        {
                                *tag = tag_index;
                        }
                        else if (idx == 0)
                        {
                                *index = std::stoi(tag_index.substr(idx + 1));
                        }
                        else
                        {
                                *tag = tag_index.substr(0, idx);
                                *index = std::stoi(tag_index.substr(idx + 1));
                        }
                        return 0;
                }

                int ParseTagIndexName(
                    const std::string& tag_index_name, std::string* tag, int* index,
                    std::string* name)
                {
                        if (tag_index_name.empty())
                        {
                                *index = -1;
                                return 0;
                        }
                        int pos1 = tag_index_name.find(":");
                        if (pos1 == -1)
                        {
                                *name = tag_index_name;
                                *index = -1;
                                return 0;
                        }
                        int pos2 = tag_index_name.substr(pos1 + 1).find(":");
                        if (pos2 == -1)
                        {
                                *index = std::stoi(tag_index_name.substr(0, pos1));
                                *name = tag_index_name.substr(pos1 + 1);
                        }
                        else
                        {
                                pos2 += (pos1 + 1);
                                *tag = tag_index_name.substr(0, pos1);
                                *index = std::stoi(tag_index_name.substr(pos1 + 1, pos2));
                                *name = tag_index_name.substr(pos2 + 1);
                        }
                        return 0;
                }

        }   // namespace utils

}   // namespace dni
