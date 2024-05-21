load("@dni//third_party:utils.bzl", "generate_export_header")

package(default_visibility = ["//visibility:public"])

licenses(["notice"])

generate_export_header(
    name = "influxdb_export",
    out = "include/influxdb_export.h",
    lib = "influxdb",
    static_define = "INFLUXDB_STATIC_DEFINE",
)

INFLUXDBCXX_VERSION = "0.7.2"

INFLUXDBCXX_USE_BOOST = False

cc_library(
    name = "influxdb-cxx",
    srcs = ["src/BoostSupport.cxx"] if INFLUXDBCXX_USE_BOOST else [
        "src/NoBoostSupport.cxx",
    ] + [
        "src/BoostSupport.h",
        "src/HTTP.cxx",
        "src/HTTP.h",
        "src/InfluxDB.cxx",
        "src/InfluxDBFactory.cxx",
        "src/LineProtocol.cxx",
        "src/LineProtocol.h",
        "src/Point.cxx",
        "src/Proxy.cxx",
        "src/UriParser.h",
    ],
    hdrs = [
        "3rd-party/date/date.h",
        ":influxdb_export",
    ] + glob(["include/*.h"]),
    copts = [
        "-Wextra",
        "-pedantic",
        "-pedantic-errors",
        "-Werror",
        "-Wshadow",
        "-Wold-style-cast",
        "-Wnull-dereference",
        "-Wnon-virtual-dtor",
        "-Woverloaded-virtual",
    ],
    defines = [
        "InfluxDB_VERSION={}".format(INFLUXDBCXX_VERSION),
    ],
    includes = ["include"],
    deps = ["@cpr"],
)
