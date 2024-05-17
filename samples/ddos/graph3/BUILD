# NOTE: Tasks are not directly used in framework,
#       so it's required to always set `alwayslink` to 1 for tasks.

licenses(["notice"])

cc_binary(
    name = "graph3",
    srcs = ["graph3.cc"],
    data = ["testdata/graph3.pbtxt"] + glob(["testdata/*.pcap"]),
    deps = [
        "//dni/framework",
        "//dni/tasks/datagram:feature_counter_task",
        "//dni/tasks/datagram:pcap_parse_task",
        "//dni/tasks/snding:attacker_ip_merge_task",
        "//dni/tasks/snding:dms_rules_dedup_task",
        "//dni/tasks/snding:net_record_merge_task",
        "//dni/tasks/snding:sip_base_merge_dedup_task",
        "@fmtlib",
        "@spdlog",
    ],
)
