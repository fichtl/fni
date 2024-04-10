# NOTE: Tasks are not directly used in framework,
#       so it's required to always set `alwayslink` to 1 for tasks.

licenses(["notice"])

###### test1 ######

cc_library(
    name = "add_input_sidedata_task",
    srcs = ["add_input_sidedata_task.cc"],
    deps = [
        "//dni/framework",
        "@spdlog",
    ],
    alwayslink = 1,
)

cc_binary(
    name = "input_sidedata",
    srcs = ["input_sidedata.cc"],
    deps = [
        ":add_input_sidedata_task",
        "//dni/framework",
        "@fmtlib",
        "@spdlog",
    ],
)

###### test2 ######

cc_library(
    name = "process_sidedata_task",
    srcs = ["process_sidedata_task.cc"],
    deps = [
        "//dni/framework",
        "@spdlog",
    ],
    alwayslink = 1,
)

cc_library(
    name = "sidedata_output_task",
    srcs = ["sidedata_output_task.cc"],
    deps = [
        "//dni/framework",
        "@spdlog",
    ],
    alwayslink = 1,
)

cc_binary(
    name = "process_sidedata",
    srcs = ["process_sidedata.cc"],
    deps = [
        ":process_sidedata_task",
        ":sidedata_output_task",
        "//dni/framework",
        "@fmtlib",
        "@spdlog",
    ],
)


