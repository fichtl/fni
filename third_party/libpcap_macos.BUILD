package(default_visibility = ["//visibility:public"])

licenses(["notice"])

cc_library(
    name = "libpcap",
    srcs = glob(["lib/libpcap.*.dylib"]),
    hdrs = glob(["include/pcap/*.h"]),
    includes = ["include/"],
    linkstatic = 1,
    visibility = ["//visibility:public"],
)
