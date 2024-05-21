package(default_visibility = ["//visibility:public"])

cc_library(
    name = "onnx",
    srcs = [
        "onnx_helper.cc",
    ],
    hdrs = [
        "onnx.h",
        "onnx_helper.h",
    ],
    deps = [
        ":onnx_model_loader",
    ],
)

cc_library(
    name = "onnx_model_loader",
    srcs = [
        "onnx_dynamic_model_loader.cc",
        "onnx_fixed_model_loader.cc",
        "onnx_model_loader.cc",
    ],
    hdrs = [
        "onnx_dynamic_model_loader.h",
        "onnx_fixed_model_loader.h",
        "onnx_model_loader.h",
    ],
    deps = [
        "//third_party:onnxruntime",
        "@fmtlib",
        "@spdlog",
    ],
)

###

proto_library(
    name = "onnx_task_proto",
    srcs = ["onnx_task.proto"],
)

cc_proto_library(
    name = "onnx_task_cc_proto",
    deps = [":onnx_task_proto"],
)

###

cc_library(
    name = "onnx_task",
    srcs = [
        "onnx_task.cc",
    ],
    hdrs = [
        "onnx_task.h",
    ],
    deps = [
        ":onnx",
        ":onnx_task_cc_proto",
        "//dni/framework",
        "@fmtlib",
        "@spdlog",
    ],
    alwayslink = 1,
)
