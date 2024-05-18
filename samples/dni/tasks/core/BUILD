load("//samples:variables.bzl", "DEPS")

_DEPS_ = DEPS + [
    "//dni/framework",
    "@spdlog",
]

cc_binary(
    name = "counter",
    srcs = ["counter.cc"],
    data = glob(["testdata/counter*.pbtxt"]),
    deps = _DEPS_ + ["//dni/tasks/core:counter_task"],
)

cc_binary(
    name = "max",
    srcs = ["max.cc"],
    data = glob(["testdata/max*.pbtxt"]),
    deps = _DEPS_ + ["//dni/tasks/core:max_task"],
)

cc_binary(
    name = "sum",
    srcs = ["sum.cc"],
    data = glob(["testdata/sum*.pbtxt"]),
    deps = _DEPS_ + ["//dni/tasks/core:sum_task"],
)

cc_binary(
    name = "threshold",
    srcs = ["threshold.cc"],
    data = glob(["testdata/threshold*.pbtxt"]),
    deps = _DEPS_ + ["//dni/tasks/core:threshold_task"],
)

cc_binary(
    name = "cond_threshold",
    srcs = ["cond_threshold.cc"],
    data = glob(["testdata/cond_threshold*.pbtxt"]),
    deps = _DEPS_ + ["//dni/tasks/core:cond_threshold_task"],
)
