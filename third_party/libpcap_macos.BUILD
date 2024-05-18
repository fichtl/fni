package(default_visibility = ["//visibility:public"])

licenses(["notice"])

cc_library(
    name = "libpcap",
    srcs = glob(["opt/libpcap/lib/libpcap.*.dylib"]),
    hdrs = glob(["opt/libpcap/include/pcap/*.h*"]),
    includes = ["opt/libpcap/include/"],
    linkstatic = 1,
    visibility = ["//visibility:public"],
)
