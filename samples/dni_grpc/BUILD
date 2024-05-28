

licenses(["none"])

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "dni_service_utils",
    srcs = ["common/utils.cpp"],
    hdrs = ["common/utils.h"],
    deps = [
        "//samples/dni_grpc/protos:dni_service_cc_grpc",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/random",
        "@com_google_absl//absl/random:distributions",
        "@spdlog",
        "@fmtlib",
    ],
)

cc_library(
    name = "dni_service_conn",
    srcs = ["common/connection.cpp"],
    hdrs = ["common/connection.h"],
    deps = [
        "//samples/dni_grpc/protos:dni_service_cc_grpc",
        ":dni_service_utils",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/random",
        "@spdlog",
        "@fmtlib",
    ],
)

###### grpc server

cc_library(
    name = "dni_server",
    srcs = [
        "server/dni_server.cpp",
        "server/dni_server.h",
    ],
    defines = ["BAZEL_BUILD"],
    deps = [
        ":dni_service_conn",
        ":dni_service_utils",
        "//samples/dni_grpc/protos:dni_service_cc_grpc",
        "@com_github_grpc_grpc//:grpc++",
        "@com_github_grpc_grpc//:grpc++_reflection",
        "//dni/framework",
        "//dni/tasks/core:max_task",
        "@spdlog",
        "@fmtlib",
    ],
)

cc_binary(
    name = "dni_server_test",
    srcs = [
        "server/server_test.cpp",
    ],
    defines = ["BAZEL_BUILD"],
    deps = [
        ":dni_server",
    ],
)

###### grpc client

cc_library(
    name = "dni_client",
    srcs = [
        "client/dni_client.cpp",
        "client/dni_client.h",
    ],
    # data = glob(["client/testdata/*.pbtxt"]),
    defines = ["BAZEL_BUILD"],
    deps = [
        ":dni_service_conn",
        ":dni_service_utils",
        "//samples/dni_grpc/protos:dni_service_cc_grpc",
        "@com_github_grpc_grpc//:grpc++",
        "@com_github_grpc_grpc//:grpc++_reflection",
        "@spdlog",
        "@fmtlib",
    ],
)

cc_binary(
    name = "client_test_2_server_stream",
    srcs = [
        "client/client_test_2_server_stream.cpp",
    ],
    data = glob(["client/testdata/*.pbtxt"]),
    defines = ["BAZEL_BUILD"],
    deps = [
        ":dni_client",
    ],
)

