licenses(["notice"])

cc_library(
    name = "DNIModelLoader",
    srcs = [
        "DNIDynamicModelLoader.cc",
        "DNIFixedModelLoader.cc",
        "DNIModelLoader.cc",
    ],
    hdrs = [
        "DNIDynamicModelLoader.h",
        "DNIFixedModelLoader.h",
        "DNIModelLoader.h",
    ],
    deps = ["//third_party:onnxruntime"],
)

cc_binary(
    name = "DNIFixedModelLoaderTest_Multi_Add",
    srcs = ["DNIFixedModelLoaderTest_Multi_Add.cc"],
    data = ["testdata/model_2_5_x_2_5_v19.onnx"],
    deps = [":DNIModelLoader"],
)

cc_binary(
    name = "DNIFixedModelLoaderTest_MatMul_Add",
    srcs = ["DNIFixedModelLoaderTest_MatMul_Add.cc"],
    data = ["testdata/model_2_5_x_5_3_v19.onnx"],
    deps = [":DNIModelLoader"],
)

cc_binary(
    name = "DNIFixedModelLoaderTest_MatMul_MatMul",
    srcs = ["DNIFixedModelLoaderTest_MatMul_MatMul.cc"],
    data = ["testdata/model_2_5_x_5_3_x_3_2_v19.onnx"],
    deps = [":DNIModelLoader"],
)

cc_binary(
    name = "DNIFixedModelLoaderTest_LogReg_Iris",
    srcs = ["DNIFixedModelLoaderTest_LogReg_Iris.cc"],
    data = ["testdata/logreg_iris_fixed_model.onnx"],
    deps = [":DNIModelLoader"],
)

cc_binary(
    name = "DNIDynamicModelLoaderTest_LogReg_Iris",
    srcs = ["DNIDynamicModelLoaderTest_LogReg_Iris.cc"],
    data = ["testdata/logreg_iris.onnx"],
    deps = [":DNIModelLoader"],
)

cc_binary(
    name = "DNIDynamicModelLoaderTest_BPNet_Iris",
    srcs = ["DNIDynamicModelLoaderTest_BPNet_Iris.cc"],
    data = ["testdata/bpnet_iris.onnx"],
    deps = [":DNIModelLoader"],
)
