

licenses(["none"])

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "hello_utils",
    srcs = ["common/utils.cpp"],
    hdrs = ["common/utils.h"],
    deps = [
        "//samples/grpc/protos:hello_cc_grpc",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/random",
        "@com_google_absl//absl/random:distributions",
        "@spdlog",
        "@fmtlib",
    ],
)

cc_library(
    name = "hello_conn",
    srcs = ["common/connection.cpp"],
    hdrs = ["common/connection.h"],
    deps = [
        "//samples/grpc/protos:hello_cc_grpc",
        ":hello_utils",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/random",
        "@spdlog",
        "@fmtlib",
    ],
)

###### grpc server

cc_library(
    name = "hello_server",
    srcs = [
        "server/proto_server.cpp",
        "server/proto_server.h",
    ],
    defines = ["BAZEL_BUILD"],
    deps = [
        ":hello_conn",
        ":hello_utils",
        "//samples/grpc/protos:hello_cc_grpc",
        "@com_github_grpc_grpc//:grpc++",
        "@com_github_grpc_grpc//:grpc++_reflection",
        "//dni/framework",
        "//dni/tasks/core:max_task",
        "//dni/tasks/core:sum_task",
        "@spdlog",
        "@fmtlib",
    ],
)

cc_binary(
    name = "hello_server_test",
    srcs = [
        "server/server_test.cpp",
    ],
    defines = ["BAZEL_BUILD"],
    deps = [
        ":hello_server",
    ],
)

###### grpc client

cc_library(
    name = "hello_client",
    srcs = [
        "client/proto_client.cpp",
        "client/proto_client.h",
    ],
    # data = glob(["client/testdata/*.pbtxt"]),
    defines = ["BAZEL_BUILD"],
    deps = [
        ":hello_conn",
        ":hello_utils",
        "//samples/grpc/protos:hello_cc_grpc",
        "@com_github_grpc_grpc//:grpc++",
        "@com_github_grpc_grpc//:grpc++_reflection",
        "@spdlog",
        "@fmtlib",
    ],
)

cc_binary(
    name = "client_test_1_unary",
    srcs = [
        "client/client_test_1_unary.cpp",
    ],
    data = glob(["client/testdata/*.pbtxt"]),
    defines = ["BAZEL_BUILD"],
    deps = [
        ":hello_client",
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
        ":hello_client",
    ],
)

cc_binary(
    name = "client_test_3_client_stream",
    srcs = [
        "client/client_test_3_client_stream.cpp",
    ],
    data = glob(["client/testdata/*.pbtxt"]),
    defines = ["BAZEL_BUILD"],
    deps = [
        ":hello_client",
    ],
)

cc_binary(
    name = "client_test_4_bidirect_stream",
    srcs = [
        "client/client_test_4_bidirect_stream.cpp",
    ],
    data = glob(["client/testdata/*.pbtxt"]),
    defines = ["BAZEL_BUILD"],
    deps = [
        ":hello_client",
    ],
)

