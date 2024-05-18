###### test1 ######

cc_library(
    name = "add_sidedata_task",
    srcs = ["add_sidedata_task.cc"],
    deps = [
        "//dni/framework",
        "@spdlog",
    ],
    alwayslink = 1,
)

cc_binary(
    name = "sidedata_test1",
    srcs = ["sidedata_test1.cc"],
    deps = [
        ":add_sidedata_task",
        "//dni/framework",
        "@fmtlib",
        "@spdlog",
    ],
)

###### test2 ######

cc_library(
    name = "addormul_sidedata_task",
    srcs = ["addormul_sidedata_task.cc"],
    deps = [
        "//dni/framework",
        "@spdlog",
    ],
    alwayslink = 1,
)

cc_library(
    name = "propagate_and_add_sidedata_task",
    srcs = ["propagate_and_add_sidedata_task.cc"],
    deps = [
        "//dni/framework",
        "@spdlog",
    ],
    alwayslink = 1,
)

cc_binary(
    name = "sidedata_test2",
    srcs = ["sidedata_test2.cc"],
    deps = [
        ":addormul_sidedata_task",
        ":propagate_and_add_sidedata_task",
        "//dni/framework",
        "@fmtlib",
        "@spdlog",
    ],
)
