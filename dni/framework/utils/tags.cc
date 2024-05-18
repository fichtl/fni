#include "dni/framework/utils/tags.h"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "dni/framework/utils/proto.h"
#include "fmt/format.h"
#include "fmt/ranges.h"
#include "spdlog/spdlog.h"

namespace dni {

        namespace utils {

                int TagMap::Initialize(const ProtoStrings& tag_index_names)
                {
                        // parse inputs
                        std::map<std::string, std::vector<std::string>> tag_to_names;
                        for (const auto& tag_index_name : tag_index_names)
                        {
                                std::string tag, name;
                                int idx = 0;
                                if (ParseTagIndexName(tag_index_name, &tag, &idx, &name))
                                {
                                        SPDLOG_ERROR("failed to parse tag_index_name");
                                        return -1;
                                }
                                SPDLOG_DEBUG(
                                    "tag_index_name:{}, tag:{}, index:{}, name:{}",
                                    tag_index_name, tag, idx, name);

                                TagData& tag_data = data_[tag];
                                std::vector<std::string>& names = tag_to_names[tag];
                                if (idx == -1)
                                {
                                        idx = tag_data.count;
                                }

                                ++tag_data.count;
                                if (names.size() <= idx)
                                        names.resize(idx + 1);
                                names[idx] = name;
                        }

                        // initialize `data_`
                        int curr = 0;
                        for (auto& item : data_)
                        {
                                TagData& tag_data = item.second;
                                const std::vector<std::string>& names =
                                    tag_to_names[item.first];
                                if (tag_data.count != names.size())
                                {
                                        SPDLOG_ERROR(
                                            "tag_data.count({}) != names.size({})",
                                            tag_data.count, names.size());
                                        return -1;
                                }

                                tag_data.base = curr;
                                curr += tag_data.count;
                        }
                        SPDLOG_DEBUG("tagmap.data:{}", data_);
                        total_names_ = curr;

                        // initialize `names_`
                        names_.reserve(total_names_);
                        for (const auto& item : data_)
                        {
                                std::vector<std::string>& names =
                                    tag_to_names[item.first];
                                names_.insert(
                                    names_.begin() + item.second.base, names.begin(),
                                    names.end());
                        }
                        SPDLOG_DEBUG("tagmap.names:{}", names_);

                        return 0;
                }

                int TagMap::NStreams(const std::string& tag)
                {
                        const auto it = data_.find(tag);
                        return it != data_.end() ? it->second.count : 0;
                }

                std::set<std::string> TagMap::Tags() const
                {
                        std::set<std::string> tags;
                        for (auto& pair : data_)
                        {
                                tags.insert(pair.first);
                        }
                        return tags;
                }

                int TagMap::FindByTagIndex(const std::string_view tag, int index)
                {
                        const auto it = data_.find(tag);
                        if (it == data_.end() || index >= it->second.count)
                                return -1;
                        return it->second.base + index;
                }

                std::shared_ptr<TagMap> NewTagMap(const ProtoStrings& fields)
                {
                        std::shared_ptr<TagMap> ret = std::make_shared<TagMap>();
                        if (ret->Initialize(fields))
                                return nullptr;
                        return std::move(ret);
                }

                bool validateTag(const std::string& tag)
                {
                        return tag.length() > 0 &&
                               std::all_of(tag.begin() + 1, tag.end(), [](char c) {
                                       return c == '_' || isalnum(c);
                               });
                }

                bool validateIndex(const std::string& idx)
                {
                        return (idx.length() == 1 && isdigit(idx[0])) ||
                               (idx.length() > 1 && isdigit(idx[0]) && idx[0] != '0' &&
                                std::all_of(idx.begin() + 1, idx.end(), [](char c) {
                                        return isdigit(c);
                                }));
                }

                // TODO: currently any name string is acceptable.
                bool validateName(const std::string& name) { return true; }

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

                // Parse `TAG:INDEX:NAME` string into tokens.
                //
                // Valid inputs:
                //   TAG:INDEX:NAME -> (TAG, INDEX, NAME)
                //   :INDEX:NAME -> ("", INDEX, NAME)
                //   TAG:NAME -> (TAG, 0, NAME)
                //   NAME -> ("", -1, NAME)
                //
                // Invalid inputs:
                //   INDEX:NAME
                //   TAG:INDEX
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
                                *tag = tag_index_name.substr(0, pos1);
                                if (!validateTag(*tag))
                                {
                                        SPDLOG_ERROR("invalid tag: {}", *tag);
                                        return -1;
                                }
                                *name = tag_index_name.substr(pos1 + 1);
                                if (!validateName(*name))
                                {
                                        SPDLOG_ERROR("invalid name: {}", *name);
                                        return -1;
                                }
                        }
                        else
                        {
                                pos2 += (pos1 + 1);
                                *tag = tag_index_name.substr(0, pos1);
                                if (!validateTag(*tag))
                                {
                                        SPDLOG_ERROR("invalid tag: {}", *tag);
                                        return -1;
                                }
                                std::string num =
                                    tag_index_name.substr(pos1 + 1, pos2 - pos1 - 1);
                                if (!validateIndex(num))
                                {
                                        SPDLOG_ERROR("invalid index: {}", num);
                                        return -1;
                                }
                                *index = std::stoi(num);
                                *name = tag_index_name.substr(pos2 + 1);
                                if (!validateName(*name))
                                {
                                        SPDLOG_ERROR("invalid name: {}", *name);
                                        return -1;
                                }
                        }
                        return 0;
                }

        }   // namespace utils

}   // namespace dni
