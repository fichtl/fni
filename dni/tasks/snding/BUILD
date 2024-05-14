# NOTE: Tasks are not directly used in framework,
#       so it's required to always set `alwayslink` to 1 for tasks.

licenses(["notice"])

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "snding_defines",
    hdrs = ["snding_defines.h"],
    deps = [
        "//dni/tasks/datagram:cidr",
        "@fmtlib",
    ],
)

cc_library(
    name = "anomaly_detection_task",
    srcs = ["anomaly_detection_task.cc"],
    deps = [
        "//dni/framework",
        "@spdlog",
    ],
    alwayslink = 1,
)

proto_library(
    name = "number_stats_task_proto",
    srcs = ["number_stats_task.proto"],
)

cc_proto_library(
    name = "number_stats_task_cc_proto",
    deps = [":number_stats_task_proto"],
)

cc_library(
    name = "number_stats_task",
    srcs = ["number_stats_task.cc"],
    deps = [
        ":number_stats_task_cc_proto",
        "//dni/framework",
        "@spdlog",
    ],
    alwayslink = 1,
)

proto_library(
    name = "protocol_stats_task_proto",
    srcs = ["protocol_stats_task.proto"],
)

cc_proto_library(
    name = "protocol_stats_task_cc_proto",
    deps = [":protocol_stats_task_proto"],
)

cc_library(
    name = "protocol_stats_task",
    srcs = ["protocol_stats_task.cc"],
    deps = [
        ":protocol_stats_task_cc_proto",
        "//dni/framework",
        "@spdlog",
    ],
    alwayslink = 1,
)

proto_library(
    name = "attacker_ip_merge_task_proto",
    srcs = ["attacker_ip_merge_task.proto"],
)

cc_proto_library(
    name = "attacker_ip_merge_task_cc_proto",
    deps = [":attacker_ip_merge_task_proto"],
)

cc_library(
    name = "attacker_ip_merge_task",
    srcs = ["attacker_ip_merge_task.cc"],
    deps = [
        ":attacker_ip_merge_task_cc_proto",
        ":snding_defines",
        "//dni/framework",
        "@spdlog",
    ],
    alwayslink = 1,
)

proto_library(
    name = "sip_base_merge_task_proto",
    srcs = ["sip_base_merge_task.proto"],
)

cc_proto_library(
    name = "sip_base_merge_task_cc_proto",
    deps = [":sip_base_merge_task_proto"],
)

cc_library(
    name = "sip_base_merge_task",
    srcs = ["sip_base_merge_task.cc"],
    deps = [
        ":sip_base_merge_task_cc_proto",
        ":snding_defines",
        "//dni/framework",
        "@spdlog",
    ],
    alwayslink = 1,
)

cc_library(
    name = "sip_base_merge_dedup_task",
    srcs = ["sip_base_merge_dedup_task.cc"],
    deps = [
        ":sip_base_merge_task_cc_proto",
        ":snding_defines",
        "//dni/framework",
        "@spdlog",
    ],
    alwayslink = 1,
)

cc_library(
    name = "net_record_merge_task",
    srcs = ["net_record_merge_task.cc"],
    deps = [
        ":snding_defines",
        "//dni/framework",
        "@spdlog",
    ],
    alwayslink = 1,
)

cc_library(
    name = "dms_rules_task",
    srcs = ["dms_rules_task.cc"],
    deps = [
        ":snding_defines",
        "//dni/framework",
        "@spdlog",
    ],
    alwayslink = 1,
)

cc_library(
    name = "dms_rules_dedup_task",
    srcs = ["dms_rules_dedup_task.cc"],
    deps = [
        ":snding_defines",
        "//dni/framework",
        "@spdlog",
    ],
    alwayslink = 1,
)
