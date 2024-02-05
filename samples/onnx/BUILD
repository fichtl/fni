licenses(["notice"])

cc_binary(
    name = "DNIModelLoader",
    srcs = ["DNIModelLoader.cc", "DNIModelLoaderTest.cc", "DNIModelLoader.h"],
    # hdrs = ["DNIModelLoader.h"],
    # copts = ["-Isamples/onnx"],
    linkopts = ["-lonnxruntime"],
    # linkstatic = 0,
)