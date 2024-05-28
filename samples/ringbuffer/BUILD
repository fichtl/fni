

licenses(["none"])

package(default_visibility = ["//visibility:public"])

###### grpc server

cc_library(
    name = "hello_server",
    srcs = [
        "reader/proto_server.cpp",
        "reader/proto_server.h",
    ],
    defines = ["BAZEL_BUILD"],
    deps = [
        "//samples/grpc:hello_conn",
        "//samples/grpc:hello_utils",
        "//samples/grpc/protos:hello_cc_grpc",
        "@com_github_grpc_grpc//:grpc++",
        "@com_github_grpc_grpc//:grpc++_reflection",
        "//dni/framework",
        "//dni/tasks/snding:rb_data_parse_task",
        "//dni/tasks/snding:attacker_ip_merge_task",
        "//dni/tasks/snding:feature_counter_task",
        "//dni/tasks/snding:sip_base_merge_dedup_task",
        "//dni/tasks/snding:net_record_merge_task",
        "//dni/tasks/snding:dms_rules_dedup_task",
        "@spdlog",
        "@fmtlib",
    ],
)

cc_binary(
    name = "hello_server_test",
    srcs = [
        "reader/server_test.cpp",
    ],
    defines = ["BAZEL_BUILD"],
    deps = [
        ":hello_server",
    ],
)

###### grpc client


cc_binary(
    name = "client_test_2_server_stream",
    srcs = [
        "writer/client_test_2_server_stream.cpp",
    ],
    data = glob(["writer/testdata/*.pbtxt"]),
    defines = ["BAZEL_BUILD"],
    deps = [
        "//samples/grpc:hello_client",
    ],
)


