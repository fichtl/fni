package(default_visibility = ["//visibility:public"])

licenses(["notice"])

cc_library(
    name = "libpcap",
    hdrs = glob(["include/pcap/*.h"]),
    linkopts = [
        "-l:libpcap.so",
    ],
    visibility = ["//visibility:public"],
)
