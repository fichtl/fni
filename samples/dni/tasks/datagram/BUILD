load("//samples:variables.bzl", "DEPS")

_DEPS_ = DEPS + [
    "//dni/framework",
    "@spdlog",
]

cc_binary(
    name = "iprange_to_cidr",
    srcs = ["iprange_to_cidr.cc"],
    deps = _DEPS_ + ["//dni/framework/formats:cidr"],
)
