# NOTE: Tasks are not directly used in framework,
#       so it's required to always set `alwayslink` to 1 for tasks.

licenses(["notice"])

cc_binary(
    name = "graph2",
    srcs = ["graph2.cc"],
    data = [
        "testdata/graph2.pbtxt",
        "testdata/test1.pcap",
    ],
    deps = [
        "//dni/framework",
        "//dni/tasks/core:max_task",
        "//dni/tasks/core:sum_task",
        "//dni/tasks/core:threshold_task",
        "//dni/tasks/datagram:feature_counter_task",
        "//dni/tasks/datagram:pcap_parse_task",
        "//dni/tasks/snding:number_stats_task",
        "//dni/tasks/snding:protocol_stats_task",
        "@fmtlib",
        "@spdlog",
    ],
)
