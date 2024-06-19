

licenses(["none"])

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "dni_service_utils",
    srcs = ["common/utils.cc"],
    hdrs = ["common/utils.h"],
    deps = [
        "//samples/ddos/realtime_with_sccm_with_grpc/protos:dni_service_cc_grpc",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/random",
        "@com_google_absl//absl/random:distributions",
        "@spdlog",
        "@fmtlib",
    ],
)

cc_library(
    name = "dni_service_conn",
    srcs = ["common/connection.cc"],
    hdrs = ["common/connection.h"],
    deps = [
        "//samples/ddos/realtime_with_sccm_with_grpc/protos:dni_service_cc_grpc",
        ":dni_service_utils",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/random",
        "@spdlog",
        "@fmtlib",
    ],
)

cc_library(
    name = "rb_data_parse",
    srcs = ["common/rb_data_parse.cc"],
    hdrs = ["common/rb_data_parse.h"],
    deps = [
        "@spdlog",
    ],
)

cc_library(
    name = "all_net_ip",
    srcs = ["common/all_net_ip.cc"],
    hdrs = ["common/all_net_ip.h"],
    deps = [
        "@influxdb-cxx",
        "@jsoncpp",
        "@spdlog",
    ],
)

###### grpc server

cc_library(
    name = "dni_server",
    srcs = [
        "server/dni_server.cc",
        "server/dni_server.h",
    ],
    defines = ["BAZEL_BUILD"],
    deps = [
        ":dni_service_conn",
        ":dni_service_utils",
        ":rb_data_parse",
        ":all_net_ip",
        "//samples/ddos/realtime_with_sccm_with_grpc/protos:dni_service_cc_grpc",
        "@com_github_grpc_grpc//:grpc++",
        "@com_github_grpc_grpc//:grpc++_reflection",
        "//dni/framework",
        "//dni/tasks/core:cond_threshold_task",
        "//dni/tasks/core:max_task",
        "//dni/tasks/core:sum_task",
        "//dni/tasks/core:threshold_task",
        "//dni/tasks/snding:anomaly_detection_task",
        "//dni/tasks/snding:attacker_ip_merge_task",
        "//dni/tasks/snding:dms_rules_dedup_task",
        "//dni/tasks/snding:feature_counter_task",
        "//dni/tasks/snding:net_record_merge_task",
        "//dni/tasks/snding:number_stats_task",
        "//dni/tasks/snding:protocol_stats_task",
        "//dni/tasks/snding:sip_base_merge_dedup_task",
        "@fmtlib",
        "@spdlog",
    ],
)

cc_binary(
    name = "dni_server_test",
    srcs = [
        "server/server_test.cc",
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
        "client/dni_client.cc",
        "client/dni_client.h",
    ],
    # data = glob(["client/testdata/*.pbtxt"]),
    defines = ["BAZEL_BUILD"],
    deps = [
        ":dni_service_conn",
        ":dni_service_utils",
        "//samples/ddos/realtime_with_sccm_with_grpc/protos:dni_service_cc_grpc",
        "@com_github_grpc_grpc//:grpc++",
        "@com_github_grpc_grpc//:grpc++_reflection",
        "@spdlog",
        "@fmtlib",
    ],
)

cc_binary(
    name = "client_test_2_server_stream",
    srcs = [
        "client/client_test_2_server_stream.cc",
    ],
    data = glob(["client/testdata/*.pbtxt"]),
    defines = ["BAZEL_BUILD"],
    deps = [
        ":dni_client",
    ],
)

