# NOTE: Tasks are not directly used in framework,
#       so it's required to always set `alwayslink` to 1 for tasks.

licenses(["notice"])

cc_library(
    name = "addone_task",
    srcs = ["addone_task.cc"],
    deps = [
        "//dni/framework",
        "@spdlog",
    ],
    alwayslink = 1,
)

cc_library(
    name = "addormul_task",
    srcs = ["addormul_task.cc"],
    deps = [
        "//dni/framework",
        "@spdlog",
    ],
    alwayslink = 1,
)

cc_library(
    name = "subordiv_task",
    srcs = ["subordiv_task.cc"],
    deps = [
        "//dni/framework",
        "@spdlog",
    ],
    alwayslink = 1,
)

cc_binary(
    name = "addone",
    srcs = ["addone.cc"],
    deps = [
        ":addone_task",
        "//dni/framework",
        "@fmtlib",
        "@spdlog",
    ],
)

cc_binary(
    name = "parallel_calc",
    srcs = ["parallel_calc.cc"],
    deps = [
        ":addormul_task",
        ":subordiv_task",
        "//dni/framework",
        "@fmtlib",
        "@spdlog",
    ],
)
