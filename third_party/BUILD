licenses(["notice"])

package(default_visibility = ["//visibility:public"])

alias(
    name = "onnxruntime",
    actual = select(
        {
            "//dni:macos_arm64": "@onnxruntime_macos_arm64//:onnxruntime",
            "//dni:macos_x86_64": "@onnxruntime_macos_x86_64//:onnxruntime",
            "//conditions:default": "@onnxruntime_linux_x64//:onnxruntime",
        },
    ),
)

alias(
    name = "libpcap",
    actual = select(
        {
            "//dni:macos_arm64": "@macos_libpcap",
            "//dni:macos_x86_64": "@macos_libpcap",
            "//conditions:default": "@linux_libpcap",
        },
    ),
)
