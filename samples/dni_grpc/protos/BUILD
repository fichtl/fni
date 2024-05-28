load("@rules_proto//proto:defs.bzl", "proto_library")
load("@com_github_grpc_grpc//bazel:cc_grpc_library.bzl", "cc_grpc_library")

licenses(["none"])

package(default_visibility = ["//visibility:public"])

proto_library(
    name = "dni_service_proto",
    srcs = ["dni_service.proto"],
)

cc_proto_library(
    name = "dni_service_cc_proto",
    deps = [":dni_service_proto"],
)

cc_grpc_library(
    name = "dni_service_cc_grpc",
    srcs = [":dni_service_proto"],
    grpc_only = True,
    deps = [":dni_service_cc_proto"],
)
