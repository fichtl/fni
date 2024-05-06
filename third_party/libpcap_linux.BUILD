package(default_visibility = ["//visibility:public"])

licenses(["notice"])

cc_library(
    name = "linux_libpcap",
    hdrs = glob([
        "include/pcap.h",
        "include/pcap/*.h",
    ]),
    linkopts = [
        "-l:libpcap.so",
    ],
    visibility = ["//visibility:public"],
)

