package(default_visibility = ["//visibility:public"])

licenses(["notice"])

cc_library(
    name = "spdlog",
    hdrs = glob([
        "include/**/*.h",
    ]),
    defines = ["SPDLOG_FMT_EXTERNAL"],
    includes = ["include"],
    visibility = ["//visibility:public"],
    deps = ["@fmtlib"],
)
