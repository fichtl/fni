# NOTE: Tasks are not directly used in framework,
#       so it's required to always set `alwayslink` to 1 for tasks.

licenses(["notice"])

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "threshold_task",
    srcs = ["threshold_task.cc"],
    deps = [
        "//dni/framework",
        "@fmtlib",
        "@spdlog",
    ],
    alwayslink = 1,
)

cc_binary(
    name = "threshold",
    srcs = ["threshold.cc"],
    deps = [
        ":threshold_task",
        "//dni/framework",
        "@fmtlib",
        "@spdlog",
    ],
)

############

cc_library(
    name = "sum_task",
    srcs = ["sum_task.cc"],
    deps = [
        "//dni/framework",
        "@fmtlib",
        "@spdlog",
    ],
    alwayslink = 1,
)

cc_binary(
    name = "sum",
    srcs = ["sum.cc"],
    deps = [
        ":sum_task",
        "//dni/framework",
        "@fmtlib",
        "@spdlog",
    ],
)

############

cc_library(
    name = "counter_task",
    srcs = ["counter_task.cc"],
    deps = [
        "//dni/framework",
        "@fmtlib",
        "@spdlog",
    ],
    alwayslink = 1,
)

cc_binary(
    name = "counter",
    srcs = ["counter.cc"],
    deps = [
        ":counter_task",
        "//dni/framework",
        "@fmtlib",
        "@spdlog",
    ],
)

############

cc_library(
    name = "cond_threshold_task",
    srcs = ["cond_threshold_task.cc"],
    deps = [
        "//dni/framework",
        "@fmtlib",
        "@spdlog",
    ],
    alwayslink = 1,
)

cc_binary(
    name = "cond_threshold",
    srcs = ["cond_threshold.cc"],
    deps = [
        ":cond_threshold_task",
        "//dni/framework",
        "@fmtlib",
        "@spdlog",
    ],
)

############

cc_library(
    name = "number_stats_task",
    srcs = ["number_stats_task.cc"],
    deps = [
        "//dni/framework",
        "@spdlog",
    ],
    alwayslink = 1,
)

cc_binary(
    name = "number_stats",
    srcs = ["number_stats.cc"],
    deps = [
        ":number_stats_task",
        "//dni/framework",
        "@fmtlib",
        "@spdlog",
    ],
)

############

cc_library(
    name = "protocol_stats_task",
    srcs = ["protocol_stats_task.cc"],
    deps = [
        "//dni/framework",
        "@spdlog",
    ],
    alwayslink = 1,
)

cc_binary(
    name = "protocol_stats",
    srcs = ["protocol_stats.cc"],
    deps = [
        ":protocol_stats_task",
        "//dni/framework",
        "@fmtlib",
        "@spdlog",
    ],
)

############

cc_library(
    name = "max_number_task",
    srcs = ["max_number_task.cc"],
    deps = [
        "//dni/framework",
        "@spdlog",
    ],
    alwayslink = 1,
)

cc_binary(
    name = "max_number",
    srcs = ["max_number.cc"],
    deps = [
        ":max_number_task",
        "//dni/framework",
        "@fmtlib",
        "@spdlog",
    ],
)

##################

cc_library(
    name = "pcap_parse_task",
    srcs = ["pcap_parse_task.cc"],
    deps = [
        "//dni/framework",
        "//third_party:libpcap",
        "@spdlog",
    ],
    alwayslink = 1,
)

cc_binary(
    name = "pcap_parse",
    srcs = ["pcap_parse.cc"],
    deps = [
        ":pcap_parse_task",
        "//dni/framework",
        "@fmtlib",
        "@spdlog",
    ],
)

##################

cc_library(
    name = "feature_counter_task",
    srcs = ["feature_counter_task.cc"],
    deps = [
        "//dni/framework",
        "@spdlog",
    ],
    alwayslink = 1,
)

cc_binary(
    name = "feature_counter",
    srcs = ["feature_counter.cc"],
    deps = [
        ":feature_counter_task",
        "//dni/framework",
        "@fmtlib",
        "@spdlog",
    ],
)

##################

cc_binary(
    name = "jsoncpp_test",
    srcs = ["jsoncpp_test.cc"],
    data = ["testdata/ceni-40nodes-topology.json"],
    deps = [
        "@jsoncpp",
    ],
)

##################

cc_binary(
    name = "cidr_merge_test_digital_format",
    srcs = ["cidr_merge_test_digital_format.cc"],
)

cc_binary(
    name = "cidr_merge_test_string_format",
    srcs = ["cidr_merge_test_string_format.cc"],
)

##
cc_library(
    name = "cidr_merge_task",
    srcs = [
        "cidr_merge_task.cc",
        "snding_defines.h",
    ],
    deps = [
        "//dni/framework",
        "@spdlog",
    ],
    alwayslink = 1,
)

cc_binary(
    name = "cidr_merge",
    srcs = [
        "cidr_merge.cc",
        "snding_defines.h",
    ],
    deps = [
        ":cidr_merge_task",
        "//dni/framework",
        "@fmtlib",
        "@spdlog",
    ],
)

##################

cc_library(
    name = "sip_base_merge_task",
    srcs = [
        "sip_base_merge_task.cc",
        "snding_defines.h",
    ],
    deps = [
        "//dni/framework",
        "@spdlog",
    ],
    alwayslink = 1,
)

cc_binary(
    name = "sip_base_merge",
    srcs = [
        "sip_base_merge.cc",
        "snding_defines.h",
        "snding_utils.cc",
        "snding_utils.h",
    ],
    deps = [
        ":sip_base_merge_task",
        "//dni/framework",
        "@fmtlib",
        "@spdlog",
    ],
)

##################

cc_library(
    name = "net_record_merge_task",
    srcs = [
        "net_record_merge_task.cc",
        "snding_defines.h",
    ],
    deps = [
        "//dni/framework",
        "@spdlog",
    ],
    alwayslink = 1,
)

cc_binary(
    name = "net_record_merge",
    srcs = [
        "net_record_merge.cc",
        "snding_defines.h",
    ],
    deps = [
        ":net_record_merge_task",
        "//dni/framework",
        "@fmtlib",
        "@spdlog",
    ],
)
