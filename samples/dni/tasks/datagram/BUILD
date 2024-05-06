# NOTE: Tasks are not directly used in framework,
#       so it's required to always set `alwayslink` to 1 for tasks.

licenses(["notice"])

package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "pcap_parse",
    srcs = ["pcap_parse.cc"],
    data = ["testdata/pcap_parse.pcap"],
    deps = [
        "//dni/framework",
        "//dni/tasks/datagram:pcap_parse_task",
        "@fmtlib",
        "@spdlog",
    ],
)

cc_binary(
    name = "feature_counter",
    srcs = ["feature_counter.cc"],
    deps = [
        "//dni/framework",
        "//dni/tasks/datagram:feature_counter_task",
        "@fmtlib",
        "@spdlog",
    ],
)

cc_binary(
    name = "iprange_to_cidr",
    srcs = ["iprange_to_cidr.cc"],
    deps = [
        "//dni/tasks/datagram:cidr",
        "@fmtlib",
        "@spdlog",
    ],
)
