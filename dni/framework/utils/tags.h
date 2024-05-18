#pragma once

#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "dni/framework/utils/proto.h"
#include "fmt/format.h"
#include "fmt/ranges.h"

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
                                // base index of tag, -1 menas uninitialized
                                int base = -1;
                                // num of streams with the same tag
                                int count = 0;
                        };

                        int Initialize(const ProtoStrings&);

                        const std::vector<std::string>& Names() const { return names_; }

                        int NStreams() { return total_names_; }
                        int NStreams(const std::string& tag);

                        std::set<std::string> Tags() const;

                        int FindByTagIndex(const std::string_view tag, int index);

                private:
                        // Map of tag - stream base/count info.
                        std::map<std::string, TagData, std::less<>> data_;

                        // Vector of stream names (indexed by id).
                        std::vector<std::string> names_;

                        int total_names_;

                        friend class fmt::formatter<dni::utils::TagMap>;
                        friend class fmt::formatter<std::shared_ptr<dni::utils::TagMap>>;
                };

                std::shared_ptr<TagMap> NewTagMap(const ProtoStrings& tag_index_names);

                bool validateTag(const std::string& tag);
                bool validateIndex(const std::string& idx);
                bool validateName(const std::string& name);
                int ParseTagIndex(
                    const std::string& tag_index, std::string* tag, int* index);
                int ParseTagIndexName(
                    const std::string& tag_index_name,
                    std::string* tag,
                    int* index,
                    std::string* name);

        }   // namespace utils

}   // namespace dni

namespace fmt {

        using TagMap = dni::utils::TagMap;

        template <>
        struct formatter<TagMap::TagData>: formatter<std::string_view> {
                auto format(const TagMap::TagData& d, format_context& ctx) const
                {
                        return format_to(ctx.out(), "[{},{})", d.base, d.base + d.count);
                }
        };

        template <>
        struct formatter<TagMap>: formatter<std::string_view> {
                auto format(const TagMap& m, format_context& ctx) const
                {
                        return format_to(
                            ctx.out(), "streams:{}, data:{}", m.names_, m.data_);
                }
        };
        template <>
        struct formatter<std::shared_ptr<TagMap>>: formatter<std::string_view> {
                auto format(const std::shared_ptr<TagMap>& m, format_context& ctx) const
                {
                        return format_to(
                            ctx.out(), "streams:{}, data:{}", m->names_, m->data_);
                }
        };

}   // namespace fmt
