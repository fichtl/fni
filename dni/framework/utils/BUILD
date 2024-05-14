package(default_visibility = ["//visibility:public"])

cc_library(
    name = "names",
    srcs = ["names.cc"],
    hdrs = ["names.h"],
    deps = [
        "//dni/framework:dni_cc_proto",
    ],
)

cc_library(
    name = "tags",
    srcs = ["tags.cc"],
    hdrs = ["tags.h"],
    deps = [
        "@com_google_protobuf//:protobuf",
        "@spdlog",
    ],
)

cc_library(
    name = "proto",
    hdrs = ["proto.h"],
    deps = ["@com_google_protobuf//:protobuf"],
)
