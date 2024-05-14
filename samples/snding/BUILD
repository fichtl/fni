# NOTE: Tasks are not directly used in framework,
#       so it's required to always set `alwayslink` to 1 for tasks.

# build all in one cmd: bazel build --config=debug //samples/snding/...

licenses(["notice"])

package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "anomaly_detection",
    srcs = ["anomaly_detection.cc"],
    deps = [
        "//dni/framework",
        "//dni/tasks/snding:anomaly_detection_task",
        "@fmtlib",
        "@spdlog",
    ],
)

############

cc_binary(
    name = "number_stats",
    srcs = ["number_stats.cc"],
    deps = [
        "//dni/framework",
        "//dni/tasks/snding:number_stats_task",
        "@fmtlib",
        "@spdlog",
    ],
)

############

cc_binary(
    name = "protocol_stats",
    srcs = ["protocol_stats.cc"],
    deps = [
        "//dni/framework",
        "//dni/tasks/snding:protocol_stats_task",
        "@fmtlib",
        "@spdlog",
    ],
)

##################

cc_binary(
    name = "sip_base_merge",
    srcs = ["sip_base_merge.cc"],
    deps = [
        "//dni/framework",
        "//dni/tasks/snding:sip_base_merge_task",
        "@spdlog",
    ],
)

##################

cc_binary(
    name = "sip_base_merge_dedup",
    srcs = ["sip_base_merge_dedup.cc"],
    deps = [
        "//dni/framework",
        "//dni/tasks/snding:sip_base_merge_dedup_task",
        "@spdlog",
    ],
)

##################

cc_binary(
    name = "net_record_merge",
    srcs = ["net_record_merge.cc"],
    deps = [
        "//dni/framework",
        "//dni/tasks/snding:net_record_merge_task",
        "@fmtlib",
        "@spdlog",
    ],
)

##################

cc_binary(
    name = "dms_rules_test",
    srcs = ["dms_rules_test.cc"],
    deps = [
        "//dni/tasks/snding:snding_defines",
        "@fmtlib",
        "@spdlog",
    ],
)

#####

cc_binary(
    name = "dms_rules_1",
    srcs = ["dms_rules_1.cc"],
    deps = [
        "//dni/framework",
        "//dni/tasks/snding:dms_rules_task",
        "//dni/tasks/snding:sip_base_merge_task",
        "@fmtlib",
        "@spdlog",
    ],
)

cc_binary(
    name = "dms_rules_2",
    srcs = ["dms_rules_2.cc"],
    deps = [
        "//dni/framework",
        "//dni/tasks/snding:dms_rules_task",
        "//dni/tasks/snding:sip_base_merge_task",
        "@fmtlib",
        "@spdlog",
    ],
)

cc_binary(
    name = "dms_rules_3",
    srcs = ["dms_rules_3.cc"],
    deps = [
        "//dni/framework",
        "//dni/tasks/snding:dms_rules_task",
        "//dni/tasks/snding:sip_base_merge_task",
        "@fmtlib",
        "@spdlog",
    ],
)

##################

cc_binary(
    name = "dms_rules_dedup_1",
    srcs = ["dms_rules_dedup_1.cc"],
    deps = [
        "//dni/framework",
        "//dni/tasks/snding:dms_rules_dedup_task",
        "//dni/tasks/snding:sip_base_merge_dedup_task",
        "@fmtlib",
        "@spdlog",
    ],
)

cc_binary(
    name = "dms_rules_dedup_2",
    srcs = ["dms_rules_dedup_2.cc"],
    deps = [
        "//dni/framework",
        "//dni/tasks/snding:dms_rules_dedup_task",
        "//dni/tasks/snding:sip_base_merge_dedup_task",
        "@fmtlib",
        "@spdlog",
    ],
)

cc_binary(
    name = "dms_rules_dedup_3",
    srcs = ["dms_rules_dedup_3.cc"],
    deps = [
        "//dni/framework",
        "//dni/tasks/snding:dms_rules_dedup_task",
        "//dni/tasks/snding:sip_base_merge_dedup_task",
        "@fmtlib",
        "@spdlog",
    ],
)
