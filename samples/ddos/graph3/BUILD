load("//samples:variables.bzl", "DEPS")

_DEPS_ = DEPS + [
    "//dni/framework",
    "@spdlog",
]

cc_binary(
    name = "graph3",
    srcs = ["graph3.cc"],
    data = ["testdata/graph3.pbtxt"] + glob(["testdata/*.pcap"]),
    deps = _DEPS_ + [
        "//dni/tasks/snding:attacker_ip_merge_task",
        "//dni/tasks/snding:dms_rules_dedup_task",
        "//dni/tasks/snding:feature_counter_task",
        "//dni/tasks/snding:net_record_merge_task",
        "//dni/tasks/snding:pcap_parse_task",
        "//dni/tasks/snding:sip_base_merge_dedup_task",
    ],
)
