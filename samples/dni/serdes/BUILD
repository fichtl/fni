############ Textproto ############

cc_binary(
    name = "textproto2graph",
    srcs = ["textproto2graph.cc"],
    data = ["testdata/graph1.pbtxt"],
    deps = ["//dni/framework"],
)

cc_binary(
    name = "graph2textproto",
    srcs = ["graph2textproto.cc"],
    data = ["testdata/graph1.pbtxt"],
    deps = ["//dni/framework"],
)

############ YAML ############

# TODO

############ JSON ############

cc_binary(
    name = "jsoncpp_test",
    srcs = ["jsoncpp_test.cc"],
    data = ["testdata/ceni-40nodes-topology.json"],
    deps = [
        "@jsoncpp",
    ],
)
