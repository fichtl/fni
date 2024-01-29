load("@rules_proto//proto:defs.bzl", "proto_library")
load("@rules_cc//cc:defs.bzl", "cc_proto_library")

package(default_visibility = ["//visibility:private"])

package_group(
    name = "dni_internal",
    packages = [
        "//dni/...",
    ],
)

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
    visibility = [":dni_internal"],
    deps = [
        "@com_google_protobuf//:any_proto",
    ],
)

cc_proto_library(
    name = "stream_handler_cc_proto",
    visibility = [":dni_internal"],
    deps = [":stream_handler_proto"],
)
