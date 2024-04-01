licenses(["notice"])

package(default_visibility = ["//visibility:public"])

alias(
    name = "onnxruntime",
    actual = select(
        {
            "@dni//dni:macos_arm64": "@onnxruntime_macos_arm64//:onnxruntime",
            "//conditions:default": "@onnxruntime_linux_x64//:onnxruntime",
        },
    ),
)
