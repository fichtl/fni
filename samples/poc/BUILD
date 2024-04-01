licenses(["notice"])

cc_library(
    name = "pipeline",
    hdrs = ["pipeline.h"],
)

cc_binary(
    name = "nop",
    srcs = ["nop.cc"],
    deps = [
        ":pipeline",
        "@taskflow",
    ],
)

cc_binary(
    name = "counter",
    srcs = ["counter.cc"],
    deps = [
        ":pipeline",
        "@taskflow",
    ],
)
