#pragma once

#include <unordered_map>
#include <unordered_set>

#include "fmt/format.h"

namespace dni {

        struct IP {
                IP() = default;
                IP(uint32_t ip, int len): ip(ip), len(len) {}

                uint32_t ip = 0;
                int len = 32;

                bool operator==(const IP& rhl) const
                {
                        if ((ip == rhl.ip) && (len == rhl.len))
                                return true;
                        else
                                return false;
                }
        };

        struct SIPBaseMergeResult {
                SIPBaseMergeResult() = default;

                int packet_total;                      // numValueSum
                std::unordered_set<uint32_t> sport;    // port number(22, 10000)
                std::unordered_set<uint32_t> dport;    // port number(22, 10000)
                std::unordered_set<uint32_t> length;   // len number(16, 1472)
                std::unordered_map<int, int>
                    protocol;   // key: proto number(1,6,17...), value: packet count of
                                // each proto
        };

        struct SIPBaseMergeNumberStats {
                SIPBaseMergeNumberStats() = default;

                std::string stat_type;                // centralize, regular, random, void
                std::unordered_set<uint32_t> value;   // std::any value;
        };

        struct SIPBaseMergeProtoStats {
                SIPBaseMergeProtoStats() = default;

                std::unordered_map<int, std::string>
                    stat_types;   // key: proto number(1,6,17...), value: stat_type of
                                  // each proto(flood, rapid, void)
        };

        // SIPBaseMergeTask final result
        struct SIPBaseMergeStats {
                SIPBaseMergeStats() = default;

                std::string hostNicSign;   // host-name#nic-name
                IP srcIP;                  // cidr, {1.2.3.0, 24}
                IP dstIP;                  // cidr, {5.6.7.8, 32}

                SIPBaseMergeNumberStats srcPort;
                SIPBaseMergeNumberStats dstPort;
                SIPBaseMergeNumberStats length;
                SIPBaseMergeProtoStats protocol;
        };
}   // namespace dni

// https://stackoverflow.com/questions/71299167/c-format-unordered-map-with-fmtjoin
// https://godbolt.org/z/s691r16Tz
namespace fmt {

        template <>
        struct formatter<dni::IP>: formatter<std::string_view> {
                auto format(const dni::IP& ip, format_context& ctx) const
                {
                        return format_to(ctx.out(), "cidr-ip {:X}/{}", ip.ip, ip.len);
                }
        };

        template <>
        struct formatter<dni::SIPBaseMergeNumberStats>: formatter<std::string_view> {
                auto format(
                    const dni::SIPBaseMergeNumberStats& stats, format_context& ctx) const
                {
                        return format_to(
                            ctx.out(), "stat_type: {}, value: {}", stats.stat_type,
                            fmt::join(stats.value, ","));
                }
        };

        template <>
        struct formatter<std::pair<const int, std::string>> {
                template <typename ParseContext>
                constexpr auto parse(ParseContext& ctx)
                {
                        return ctx.begin();
                }

                template <typename FormatContext>
                auto format(
                    const std::pair<const int, std::string>& fp, FormatContext& ctx) const
                {
                        return format_to(
                            ctx.out(), "\n             {}:{}", fp.first, fp.second);
                }
        };

        template <>
        struct formatter<dni::SIPBaseMergeProtoStats>: formatter<std::string_view> {
                auto format(
                    const dni::SIPBaseMergeProtoStats& stats, format_context& ctx) const
                {
                        auto j = fmt::join(
                            stats.stat_types.begin(), stats.stat_types.end(), ", ");
                        return format_to(ctx.out(), "stat_types: {}", std::move(j));
                }
        };

        template <>
        struct formatter<dni::SIPBaseMergeStats>: formatter<std::string_view> {
                auto format(
                    const dni::SIPBaseMergeStats& stats, format_context& ctx) const
                {
                        return format_to(
                            ctx.out(),
                            "hostNicSign: {}\n, srcIP: {}\n, dstIP: {}\n, srcPort: {}\n, "
                            "dstPort: {}\n, length: {}\n, protocol: {}\n",
                            stats.hostNicSign, stats.srcIP, stats.dstIP, stats.srcPort,
                            stats.dstPort, stats.length, stats.protocol);
                }
        };

}   // namespace fmt
