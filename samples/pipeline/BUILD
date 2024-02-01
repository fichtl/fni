licenses(["notice"])

cc_library(
    name = "pipeline",
    hdrs = ["pipeline.h"],
)

cc_binary(
    name = "simple",
    srcs = ["simple.cc"],
    deps = [
        ":pipeline",
        "//third_party:taskflow",
    ],
)

cc_binary(
    name = "counter",
    srcs = ["counter.cc"],
    deps = [
        ":pipeline",
        "//third_party:taskflow",
    ],
)
