# NOTE: Tasks are not directly used in framework,
#       so it's required to always set `alwayslink` to 1 for tasks.

licenses(["notice"])

cc_binary(
    name = "graph1_a",
    srcs = ["graph1_a.cc"],
    data = ["testdata/graph1.pbtxt"],
    deps = [
        "//dni/framework",
        "//dni/tasks/core:threshold_task",
        "@fmtlib",
        "@spdlog",
    ],
)

cc_binary(
    name = "graph1_b",
    srcs = ["graph1_b.cc"],
    data = ["testdata/graph1.pbtxt"],
    deps = [
        "//dni/framework",
        "//dni/tasks/core:cond_threshold_task",
        "//dni/tasks/core:threshold_task",
        "@fmtlib",
        "@spdlog",
    ],
)

cc_binary(
    name = "graph1_c",
    srcs = ["graph1_c.cc"],
    data = ["testdata/graph1.pbtxt"],
    deps = [
        "//dni/framework",
        "//dni/tasks/core:cond_threshold_task",
        "//dni/tasks/core:sum_task",
        "//dni/tasks/core:threshold_task",
        "//dni/tasks/snding:anomaly_detection_task",
        "@fmtlib",
        "@spdlog",
    ],
)

cc_binary(
    name = "graph1",
    srcs = ["graph1.cc"],
    data = ["testdata/graph1.pbtxt"],
    deps = [
        "//dni/framework",
        "//dni/tasks/core:cond_threshold_task",
        "//dni/tasks/core:sum_task",
        "//dni/tasks/core:threshold_task",
        "//dni/tasks/snding:anomaly_detection_task",
        "@fmtlib",
        "@spdlog",
    ],
)
