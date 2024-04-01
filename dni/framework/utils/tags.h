#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "google/protobuf/repeated_ptr_field.h"

namespace dni {

        namespace utils {
                // A TagMap stores streams' indexes by tag and index, as well as their
                // names in a vector.
                class TagMap {
                public:
                        TagMap() {}

                        struct TagData {
                                TagData() {}
                                TagData(int base, int count): base(base), count(count) {}
                                // base index of tag
                                int base = 0;
                                // num of streams with the same tag
                                int count = 0;
                        };

                        int Initialize(
                            const google::protobuf::RepeatedPtrField<std::string>&
                                tag_names);

                        const std::vector<std::string>& Names() const { return names_; }

                        const std::unordered_map<std::string, TagData>& Map() const
                        {
                                return data_;
                        }

                        int NStreams();
                        int NStreams(const std::string& tag);

                        int FindByTagIndex(const std::string& tag, int index);

                private:
                        // Map of tag - stream base/count info.
                        std::unordered_map<std::string, TagData> data_;

                        // Vector of stream names (indexed by id).
                        std::vector<std::string> names_;

                        int total_names_;
                };

                std::shared_ptr<TagMap> NewTagMap(
                    const google::protobuf::RepeatedPtrField<std::string>&
                        tag_index_names);

                int ParseTagIndex(
                    const std::string& tag_index, std::string* tag, int* index);
                int ParseTagIndexName(
                    const std::string& tag_index_name,
                    std::string* tag,
                    int* index,
                    std::string* name);

        }   // namespace utils

}   // namespace dni
