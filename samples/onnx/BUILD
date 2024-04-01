licenses(["notice"])

cc_binary(
    name = "DNIFixedModelLoaderTest_Multi_Add",
    srcs = ["DNIFixedModelLoaderTest_Multi_Add.cc", "DNIModelLoader.cc", "DNIModelLoader.h", "DNIFixedModelLoader.cc", "DNIFixedModelLoader.h"],
    linkopts = ["-lonnxruntime"],
)

cc_binary(
    name = "DNIFixedModelLoaderTest_MatMul_Add",
    srcs = ["DNIFixedModelLoaderTest_MatMul_Add.cc", "DNIModelLoader.cc", "DNIModelLoader.h", "DNIFixedModelLoader.cc", "DNIFixedModelLoader.h"],
    linkopts = ["-lonnxruntime"],
)

cc_binary(
    name = "DNIFixedModelLoaderTest_MatMul_MatMul",
    srcs = ["DNIFixedModelLoaderTest_MatMul_MatMul.cc", "DNIModelLoader.cc", "DNIModelLoader.h", "DNIFixedModelLoader.cc", "DNIFixedModelLoader.h"],
    linkopts = ["-lonnxruntime"],
)

cc_binary(
    name = "DNIFixedModelLoaderTest_LogReg_Iris",
    srcs = ["DNIFixedModelLoaderTest_LogReg_Iris.cc", "DNIModelLoader.cc", "DNIModelLoader.h", "DNIFixedModelLoader.cc", "DNIFixedModelLoader.h"],
    deps = ["//third_party:onnxruntime"],
)

cc_binary(
    name = "DNIDynamicModelLoaderTest_LogReg_Iris",
    srcs = ["DNIDynamicModelLoaderTest_LogReg_Iris.cc", "DNIModelLoader.cc", "DNIModelLoader.h", "DNIDynamicModelLoader.cc", "DNIDynamicModelLoader.h"],
    linkopts = ["-lonnxruntime"],
)

cc_binary(
    name = "DNIDynamicModelLoaderTest_BPNet_Iris",
    srcs = ["DNIDynamicModelLoaderTest_BPNet_Iris.cc", "DNIModelLoader.cc", "DNIModelLoader.h", "DNIDynamicModelLoader.cc", "DNIDynamicModelLoader.h"],
    linkopts = ["-lonnxruntime"],
)
