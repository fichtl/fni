load("@rules_cc//cc:defs.bzl", "cc_proto_library")
load("@rules_proto//proto:defs.bzl", "proto_library")

package(default_visibility = ["//visibility:public"])

# package_group(
#     name = "dni_internal",
#     packages = [
#         "//dni/...",
#     ],
# )

proto_library(
    name = "dni_proto",
    srcs = ["dni.proto"],
    visibility = ["//visibility:public"],
    deps = [
        ":stream_handler_proto",
        "@com_google_protobuf//:any_proto",
    ],
)

cc_proto_library(
    name = "dni_cc_proto",
    visibility = ["//visibility:public"],
    deps = [":dni_proto"],
)

proto_library(
    name = "stream_handler_proto",
    srcs = ["stream_handler.proto"],
    visibility = ["//visibility:public"],
    deps = [
        "@com_google_protobuf//:any_proto",
    ],
)

cc_proto_library(
    name = "stream_handler_cc_proto",
    visibility = ["//visibility:public"],
    deps = [":stream_handler_proto"],
)

cc_library(
    name = "framework",
    hdrs = ["framework.h"],
    deps = [
        ":datum",
        ":graph",
        ":graph_config",
        ":input_stream",
        ":node_config",
        ":output_side_data",
        ":output_stream",
        ":output_stream_manager",
        ":task",
        ":task_context",
    ],
)

cc_library(
    name = "graph",
    srcs = ["graph.cc"],
    hdrs = ["graph.h"],
    deps = [
        ":datum",
        ":dni_cc_proto",
        ":graph_config",
        ":graph_output_stream",
        ":input_stream_manager",
        ":node",
        ":output_side_data",
        ":output_stream_manager",
        "//dni/framework/utils:tags",
        "@com_google_protobuf//:protobuf",
        "@fmtlib",
        "@spdlog",
        "@taskflow",
    ],
)

cc_library(
    name = "graph_config",
    srcs = ["graph_config.cc"],
    hdrs = ["graph_config.h"],
    deps = [
        ":dni_cc_proto",
        ":dtype",
        ":node_config",
        "//dni/framework/utils:names",
        "//dni/framework/utils:proto",
        "//dni/framework/utils:tags",
        "@fmtlib",
    ],
)

cc_library(
    name = "node_config",
    srcs = ["node_config.cc"],
    hdrs = ["node_config.h"],
    deps = [
        ":dni_cc_proto",
        ":dtype",
        "//dni/framework/utils:tags",
        "@fmtlib",
        "@spdlog",
    ],
)

cc_library(
    name = "node",
    srcs = ["node.cc"],
    hdrs = ["node.h"],
    deps = [
        ":datum",
        ":default_input_stream_handler",
        ":dni_cc_proto",
        ":dtype",
        ":graph_config",
        ":input_stream_handler",
        ":input_stream_manager",
        ":node_config",
        ":output_stream_handler",
        ":output_stream_manager",
        ":task",
        ":task_context",
        ":task_state",
        "//dni/framework/utils:names",
        "@fmtlib",
        "@spdlog",
        "@taskflow",
    ],
)

# TODO: resolve circular dependency
# cc_library(
#     name = "task_register",
#     # srcs = ["task_register.cc"],
#     hdrs = ["task_register.h"],
#     deps = [
#         ":task",
#     ],
# )

cc_library(
    name = "task",
    srcs = ["task.cc"],
    hdrs = [
        "task.h",
        "task_register.h",
    ],
    deps = [
        ":task_context",
    ],
)

cc_library(
    name = "task_context_manager",
    srcs = ["task_context_manager.cc"],
    hdrs = ["task_context_manager.h"],
    deps = [
        ":task_context",
        ":task_state",
        "//dni/framework/utils:tags",
        "@fmtlib",
    ],
)

cc_library(
    name = "task_context",
    # srcs = ["task_context.cc"],
    hdrs = ["task_context.h"],
    deps = [
        ":datum",
        ":input_side_data",
        ":input_stream",
        ":output_side_data",
        ":output_stream",
        ":task_state",
        ":timestamp",
        "@fmtlib",
    ],
)

cc_library(
    name = "task_state",
    # srcs = ["task_state.cc"],
    hdrs = ["task_state.h"],
    deps = [
        ":datum",
        ":dni_cc_proto",
        ":input_side_data",
        ":output_side_data",
        "@fmtlib",
    ],
)

cc_library(
    name = "input_side_data",
    srcs = ["input_side_data.cc"],
    hdrs = ["input_side_data.h"],
    deps = [
        ":datum",
        ":dtype",
        "@spdlog",
    ],
)

cc_library(
    name = "output_side_data",
    srcs = ["output_side_data.cc"],
    hdrs = ["output_side_data.h"],
    deps = [
        ":datum",
        ":dtype",
        ":input_side_data",
    ],
)

cc_library(
    name = "graph_output_stream",
    srcs = ["graph_output_stream.cc"],
    hdrs = ["graph_output_stream.h"],
    deps = [
        ":input_stream_handler",
        ":input_stream_manager",
        ":output_stream_manager",
        "//dni/framework/utils:tags",
    ],
)

cc_library(
    name = "output_stream_handler",
    srcs = ["output_stream_handler.cc"],
    hdrs = ["output_stream_handler.h"],
    deps = [
        ":output_stream",
        ":output_stream_manager",
        ":task_context",
        ":task_context_manager",
        "//dni/framework/utils:tags",
        "@fmtlib",
        "@spdlog",
    ],
)

cc_library(
    name = "output_stream_manager",
    srcs = ["output_stream_manager.cc"],
    hdrs = ["output_stream_manager.h"],
    deps = [
        ":datum",
        ":dtype",
        ":input_stream_handler",
        ":output_stream",
        "//dni/framework/utils:tags",
    ],
)

cc_library(
    name = "output_stream",
    srcs = ["output_stream.cc"],
    hdrs = ["output_stream.h"],
    deps = [
        ":datum",
        ":dtype",
        ":timestamp",
        "@fmtlib",
    ],
)

cc_library(
    name = "default_input_stream_handler",
    srcs = ["default_input_stream_handler.cc"],
    hdrs = ["default_input_stream_handler.h"],
    deps = [
        ":input_stream_handler",
    ],
)

cc_library(
    name = "input_stream_handler",
    srcs = [
        "input_stream_handler.cc",
    ],
    hdrs = [
        "input_stream_handler.h",
    ],
    deps = [
        ":datum",
        ":input_stream_manager",
        ":task_context",
        ":task_context_manager",
        "//dni/framework/utils:tags",
        "@fmtlib",
        "@spdlog",
    ],
)

cc_library(
    name = "input_stream_manager",
    srcs = ["input_stream_manager.cc"],
    hdrs = ["input_stream_manager.h"],
    deps = [
        ":datum",
        ":dtype",
        ":input_stream",
        ":timestamp",
        "@fmtlib",
    ],
)

cc_library(
    name = "input_stream",
    srcs = ["input_stream.cc"],
    hdrs = ["input_stream.h"],
    deps = [
        ":datum",
        "//dni/framework/utils:tags",
    ],
)

cc_library(
    name = "datum",
    srcs = ["datum.cc"],
    hdrs = ["datum.h"],
    deps = [
        "@fmtlib",
        "@spdlog",
    ],
)

cc_library(
    name = "dtype",
    hdrs = ["dtype.h"],
    deps = [
        ":datum",
        "//dni/framework/utils:tags",
    ],
)

cc_library(
    name = "timestamp",
    # srcs = ["timestamp.cc"],
    hdrs = ["timestamp.h"],
)
