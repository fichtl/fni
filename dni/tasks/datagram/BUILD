licenses(["notice"])

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "cidr",
    srcs = ["cidr.cc"],
    hdrs = ["cidr.h"],
    deps = [
        "@fmtlib",
        "@spdlog",
    ],
)

cc_library(
    name = "daq",
    srcs = ["daq.cc"],
    hdrs = ["daq.h"],
    deps = [
        "//third_party:libpcap",
        "@spdlog",
    ],
)

cc_library(
    name = "datagram_to_vector_task",
    srcs = ["datagram_to_vector_task.cc"],
    deps = [
        "//dni/framework",
        "@fmtlib",
        "@spdlog",
    ],
    alwayslink = 1,
)

cc_library(
    name = "pcap_parse_task",
    srcs = ["pcap_parse_task.cc"],
    deps = [
        ":daq",
        "//dni/framework",
        "@spdlog",
    ],
    alwayslink = 1,
)

proto_library(
    name = "feature_counter_task_proto",
    srcs = ["feature_counter_task.proto"],
)

cc_proto_library(
    name = "feature_counter_task_cc_proto",
    deps = [":feature_counter_task_proto"],
)

cc_library(
    name = "feature_counter_task",
    srcs = ["feature_counter_task.cc"],
    deps = [
        ":feature_counter_task_cc_proto",
        "//dni/framework",
        "@spdlog",
    ],
    alwayslink = 1,
)
