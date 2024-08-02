package(default_visibility = ["//visibility:public"])

cc_library(
    name = "onnx_two_dimesion_task",
    srcs = ["onnx_two_dimesion_task.cc"],
    deps = [
        "//dni/framework",
        "//dni/tasks/onnx:onnx_task",
        "@spdlog",
    ],
    alwayslink = 1,
)

cc_binary(
    name = "onnx_two_dimesion_1_multi_add",
    srcs = ["onnx_two_dimesion_1_multi_add.cc"],
    data = ["testdata/model_2_5_x_2_5_v19.onnx"],
    deps = [":onnx_two_dimesion_task"],
)

cc_binary(
    name = "onnx_two_dimesion_2_matmul_add",
    srcs = ["onnx_two_dimesion_2_matmul_add.cc"],
    data = ["testdata/model_2_5_x_5_3_v19.onnx"],
    deps = [":onnx_two_dimesion_task"],
)

cc_binary(
    name = "onnx_two_dimesion_3_matmul_matmul",
    srcs = ["onnx_two_dimesion_3_matmul_matmul.cc"],
    data = ["testdata/model_2_5_x_5_3_x_3_2_v19.onnx"],
    deps = [":onnx_two_dimesion_task"],
)

cc_binary(
    name = "onnx_two_dimesion_4_bpnet_iris",
    srcs = ["onnx_two_dimesion_4_bpnet_iris.cc"],
    data = ["testdata/bpnet_iris.onnx"],
    deps = [":onnx_two_dimesion_task"],
)

#############

cc_library(
    name = "onnx_logreg_task",
    srcs = ["onnx_logreg_task.cc"],
    hdrs = ["onnx_defines.h"],
    deps = [
        "//dni/framework",
        "//dni/tasks/onnx:onnx_task",
        "@spdlog",
    ],
    alwayslink = 1,
)

cc_binary(
    name = "onnx_logreg_iris_1_fixed_model",
    srcs = ["onnx_logreg_iris_1_fixed_model.cc"],
    data = ["testdata/logreg_iris_fixed_model.onnx"],
    deps = [":onnx_logreg_task"],
)

cc_binary(
    name = "onnx_logreg_iris_2_dynamic_model",
    srcs = ["onnx_logreg_iris_2_dynamic_model.cc"],
    data = ["testdata/logreg_iris.onnx"],
    deps = [":onnx_logreg_task"],
)

###################

cc_library(
    name = "onnx_snding_mfr_task",
    srcs = ["onnx_snding_mfr_task.cc"],
    deps = [
        "//dni/framework",
        "//dni/tasks/onnx:onnx_task",
        "@spdlog",
    ],
    alwayslink = 1,
)

cc_binary(
    name = "onnx_snding_mfr_1_fixed_model",
    srcs = ["onnx_snding_mfr_1_fixed_model.cc"],
    data = ["testdata/snding_finetuned_model_200_20240724163131.onnx"],
    deps = [
        ":onnx_snding_mfr_task",
        "//third_party:libpcap",
    ],
)
