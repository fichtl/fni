# NOTE: Tasks are not directly used in framework,
#       so it's required to always set `alwayslink` to 1 for tasks.

licenses(["notice"])

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "transparent_task",
    srcs = ["transparent_task.cc"],
    deps = [
        "//dni/framework",
        "@fmtlib",
        "@spdlog",
    ],
    alwayslink = 1,
)

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

cc_library(
    name = "max_task",
    srcs = ["max_task.cc"],
    deps = [
        "//dni/framework",
        "@spdlog",
    ],
    alwayslink = 1,
)
