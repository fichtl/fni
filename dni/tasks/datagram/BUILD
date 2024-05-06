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
        "//dni/framework",
        "//third_party:libpcap",
        "@spdlog",
    ],
    alwayslink = 1,
)

cc_library(
    name = "feature_counter_task",
    srcs = ["feature_counter_task.cc"],
    deps = [
        "//dni/framework",
        "@spdlog",
    ],
    alwayslink = 1,
)
