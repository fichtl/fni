load("//samples:variables.bzl", "DEPS")

_DEPS_ = DEPS + [
    "//dni/framework",
    "@spdlog",
]

cc_binary(
    name = "graph2",
    srcs = ["graph2.cc"],
    data = [
        "testdata/graph2.pbtxt",
        "testdata/test1.pcap",
    ],
    deps = _DEPS_ + [
        "//dni/tasks/core:max_task",
        "//dni/tasks/core:sum_task",
        "//dni/tasks/core:threshold_task",
        "//dni/tasks/snding:feature_counter_task",
        "//dni/tasks/snding:number_stats_task",
        "//dni/tasks/snding:pcap_parse_task",
        "//dni/tasks/snding:protocol_stats_task",
    ],
)
