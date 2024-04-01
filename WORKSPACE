workspace(name = "dni")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "bazel_skylib",
    sha256 = "74d544d96f4a5bb630d465ca8bbcfe231e3594e5aae57e1edbf17a6eb3ca2506",
    urls = [
        "https://storage.googleapis.com/mirror.tensorflow.org/github.com/bazelbuild/bazel-skylib/releases/download/1.3.0/bazel-skylib-1.3.0.tar.gz",
        "https://github.com/bazelbuild/bazel-skylib/releases/download/1.3.0/bazel-skylib-1.3.0.tar.gz",
    ],
)

http_archive(
    name = "rules_cc",
    strip_prefix = "rules_cc-2f8c04c04462ab83c545ab14c0da68c3b4c96191",
    # The commit can be updated if the build passes. Last updated 6/23/22.
    urls = ["https://github.com/bazelbuild/rules_cc/archive/2f8c04c04462ab83c545ab14c0da68c3b4c96191.zip"],
)

load("@rules_cc//cc:repositories.bzl", "rules_cc_dependencies")

rules_cc_dependencies()

http_archive(
    name = "rules_foreign_cc",
    sha256 = "2a4d07cd64b0719b39a7c12218a3e507672b82a97b98c6a89d38565894cf7c51",
    strip_prefix = "rules_foreign_cc-0.9.0",
    url = "https://github.com/bazelbuild/rules_foreign_cc/archive/refs/tags/0.9.0.tar.gz",
)

load("@rules_foreign_cc//foreign_cc:repositories.bzl", "rules_foreign_cc_dependencies")

rules_foreign_cc_dependencies()

http_archive(
    name = "zlib",
    build_file = "@//third_party:zlib.BUILD",
    patch_args = [
        "-p1",
    ],
    # patches = [
    #     "@//third_party:zlib.diff",
    # ],
    sha256 = "b3a24de97a8fdbc835b9833169501030b8977031bcb54b3b3ac13740f846ab30",
    strip_prefix = "zlib-1.2.13",
    url = "http://zlib.net/fossils/zlib-1.2.13.tar.gz",
)

http_archive(
    name = "com_google_protobuf",
    patch_args = [
        "-p1",
    ],
    # patches = [
    #     "@//third_party:com_google_protobuf_fixes.diff",
    # ],
    sha256 = "87407cd28e7a9c95d9f61a098a53cf031109d451a7763e7dd1253abf8b4df422",
    strip_prefix = "protobuf-3.19.1",
    urls = ["https://github.com/protocolbuffers/protobuf/archive/v3.19.1.tar.gz"],
)

http_archive(
    name = "rules_proto",
    sha256 = "602e7161d9195e50246177e7c55b2f39950a9cf7366f74ed5f22fd45750cd208",
    strip_prefix = "rules_proto-97d8af4dc474595af3900dd85cb3a29ad28cc313",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/rules_proto/archive/97d8af4dc474595af3900dd85cb3a29ad28cc313.tar.gz",
        "https://github.com/bazelbuild/rules_proto/archive/97d8af4dc474595af3900dd85cb3a29ad28cc313.tar.gz",
    ],
)

load("@rules_proto//proto:repositories.bzl", "rules_proto_dependencies", "rules_proto_toolchains")

rules_proto_dependencies()

rules_proto_toolchains()

http_archive(
    name = "fmtlib",
    build_file = "@//third_party:fmtlib.BUILD",
    sha256 = "1250e4cc58bf06ee631567523f48848dc4596133e163f02615c97f78bab6c811",
    strip_prefix = "fmt-10.2.1",
    url = "https://github.com/fmtlib/fmt/archive/refs/tags/10.2.1.tar.gz",
)

http_archive(
    name = "spdlog",
    build_file = "@//third_party:spdlog.BUILD",
    sha256 = "534f2ee1a4dcbeb22249856edfb2be76a1cf4f708a20b0ac2ed090ee24cfdbc9",
    strip_prefix = "spdlog-1.13.0",
    url = "https://github.com/gabime/spdlog/archive/refs/tags/v1.13.0.tar.gz",
)

http_archive(
    name = "taskflow",
    build_file = "@//third_party:taskflow.BUILD",
    sha256 = "5a1cd9cf89f93a97fcace58fd73ed2fc8ee2053bcb43e047acb6bc121c3edf4c",
    strip_prefix = "taskflow-3.6.0",
    url = "https://github.com/taskflow/taskflow/archive/refs/tags/v3.6.0.tar.gz",
)

http_archive(
    name = "onnxruntime_macos_arm64",
    build_file = "@//third_party:onnxruntime_macos.BUILD",
    sha256 = "89566f424624a7ad9a7d9d5e413c44b9639a994d7171cf409901d125b16e2bb3",
    strip_prefix = "onnxruntime-osx-arm64-1.17.1",
    url = "https://github.com/microsoft/onnxruntime/releases/download/v1.17.1/onnxruntime-osx-arm64-1.17.1.tgz",
)

http_archive(
    name = "onnxruntime_linux_x64",
    build_file = "@//third_party:onnxruntime_linux.BUILD",
    sha256 = "89b153af88746665909c758a06797175ae366280cbf25502c41eb5955f9a555e",
    strip_prefix = "onnxruntime-linux-x64-1.17.1",
    url = "https://github.com/microsoft/onnxruntime/releases/download/v1.17.1/onnxruntime-linux-x64-1.17.1.tgz",
)
