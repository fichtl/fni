package(default_visibility = ["//visibility:public"])

licenses(["notice"])

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
            "//dni:macos_arm64": "@libpcap_macos_arm64//:libpcap",
            "//dni:macos_x86_64": "@libpcap_macos_x86_64//:libpcap",
            "//conditions:default": "@libpcap_linux//:libpcap",
        },
    ),
)
