#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "dni/tasks/datagram/cidr.h"
#include "fmt/format.h"

namespace dni {

        namespace snding {

                struct SIPBaseMergeResult {
                        SIPBaseMergeResult() = default;

                        int packet_total;                      // numValueSum
                        std::unordered_set<uint32_t> sport;    // port number(22, 10000)
                        std::unordered_set<uint32_t> dport;    // port number(22, 10000)
                        std::unordered_set<uint32_t> length;   // len number(16, 1472)

                        // key: proto number(1,6,17...), value: packet count of each proto
                        std::unordered_map<int, int> protocol;

                        std::unordered_set<uint32_t> dstIP;   // cidr set, masklen=32
                };

                struct SIPBaseMergeNumberStats {
                        SIPBaseMergeNumberStats() = default;

                        std::string stat_type;   // centralize, regular, random, void
                        std::unordered_set<uint32_t> value;
                };

                struct SIPBaseMergeProtoStats {
                        SIPBaseMergeProtoStats() = default;

                        // key: proto number(1,6,17...),
                        // value: stat_type of each proto(flood, rapid, void)
                        std::unordered_map<int, std::string> stat_types;
                };

                // SIPBaseMergeTask final result
                struct SIPBaseMergeStats {
                        SIPBaseMergeStats() = default;

                        std::string hostNicSign;   // host-name#nic-name

                        bool isSrcIPRandom = false;
                        dni::CIDR srcIP;   // cidr, {1.2.3.0, 24}
                        // std::unordered_set<dni::CIDR> dstIP;   // cidr set, masklen=32
                        SIPBaseMergeNumberStats dstIP;
                        SIPBaseMergeNumberStats srcPort;
                        SIPBaseMergeNumberStats dstPort;
                        SIPBaseMergeNumberStats length;
                        SIPBaseMergeProtoStats protocol;
                };

                struct AttackerIPMergeResult {
                        AttackerIPMergeResult() = default;

                        // after cidr merge
                        std::vector<dni::CIDR> attackerIPs;

                        // can not make cidr merge, random
                        bool containRandomAttack = false;
                        std::unordered_set<uint32_t> randomIPs;
                };

                struct DMSRule {
                        DMSRule() = default;

                        std::string hostNicSign;   // host-name#nic-name
                        dni::CIDR srcIP;           // cidr, {1.2.3.0, 24}
                        dni::CIDR dstIP;           // cidr, {5.6.7.8, 32}

                        int sPort = -1;      // if -1, do not config sport
                        int dPort = -1;      // if -1, do not config dport
                        int protocol = -1;   // if -1, do not config protocol

                        std::string action = "drop";   // drop/limit
                        std::string limitMode;
                        uint64_t limitMaxValue;
                };

        }   // namespace snding

}   // namespace dni

// https://stackoverflow.com/questions/71299167/c-format-unordered-map-with-fmtjoin
// https://godbolt.org/z/s691r16Tz
namespace fmt {

        template <>
        struct formatter<dni::snding::AttackerIPMergeResult>
            : formatter<std::string_view> {
                auto format(
                    const dni::snding::AttackerIPMergeResult& result,
                    format_context& ctx) const
                {
                        return format_to(
                            ctx.out(),
                            "attackerIPs: {}\n, containRandomAttack: {}\n, randomIPs: "
                            "{}\n",
                            fmt::join(result.attackerIPs, ","),
                            result.containRandomAttack, fmt::join(result.randomIPs, ","));
                }
        };

        template <>
        struct formatter<dni::snding::DMSRule>: formatter<std::string_view> {
                auto format(const dni::snding::DMSRule& rule, format_context& ctx) const
                {
                        return format_to(
                            ctx.out(),
                            "hostNicSign: {}\n, srcIP: {}\t, dstIP: {}\n,"
                            "sPort: {}\t, dPort: {}\t, protocol: {}\n"
                            "action: {}\t, limitMode: {}\t, limitMaxValue: {}\n\n",
                            rule.hostNicSign, rule.srcIP, rule.dstIP, rule.sPort,
                            rule.dPort, rule.protocol, rule.action, rule.limitMode,
                            rule.limitMaxValue);
                }
        };

        template <>
        struct formatter<dni::snding::SIPBaseMergeNumberStats>
            : formatter<std::string_view> {
                auto format(
                    const dni::snding::SIPBaseMergeNumberStats& stats,
                    format_context& ctx) const
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
        struct formatter<dni::snding::SIPBaseMergeProtoStats>
            : formatter<std::string_view> {
                auto format(
                    const dni::snding::SIPBaseMergeProtoStats& stats,
                    format_context& ctx) const
                {
                        auto j = fmt::join(
                            stats.stat_types.begin(), stats.stat_types.end(), ", ");
                        return format_to(ctx.out(), "stat_types: {}", std::move(j));
                }
        };

        template <>
        struct formatter<dni::snding::SIPBaseMergeStats>: formatter<std::string_view> {
                auto format(
                    const dni::snding::SIPBaseMergeStats& stats,
                    format_context& ctx) const
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
