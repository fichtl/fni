load("//samples:variables.bzl", "DEPS")

_DEPS_ = DEPS + [
    "//dni/framework",
    "@spdlog",
]

##################

cc_binary(
    name = "pcap_parse",
    srcs = ["pcap_parse.cc"],
    data = glob(["testdata/pcap_parse*"]),
    deps = _DEPS_ + ["//dni/tasks/snding:pcap_parse_task"],
)

cc_binary(
    name = "pcap_parse_vlan_id",
    srcs = ["pcap_parse_vlan_id.cc"],
    data = glob(["testdata/pcap_parse*"]),
    deps = _DEPS_ + ["//dni/tasks/snding:pcap_parse_task"],
)

cc_binary(
    name = "pcap_parse_stp",
    srcs = ["pcap_parse_stp.cc"],
    data = glob(["testdata/pcap_parse*"]),
    deps = _DEPS_ + ["//dni/tasks/snding:pcap_parse_task"],
)

##################

cc_binary(
    name = "feature_counter",
    srcs = ["feature_counter.cc"],
    data = glob(["testdata/feature_counter*"]),
    deps = _DEPS_ + ["//dni/tasks/snding:feature_counter_task"],
)

##################

cc_binary(
    name = "anomaly_detection",
    srcs = ["anomaly_detection.cc"],
    data = glob(["testdata/anomaly_detection*"]),
    deps = _DEPS_ + ["//dni/tasks/snding:anomaly_detection_task"],
)

##################

cc_binary(
    name = "number_stats",
    srcs = ["number_stats.cc"],
    data = glob(["testdata/number_stats*"]),
    deps = _DEPS_ + ["//dni/tasks/snding:number_stats_task"],
)

##################

cc_binary(
    name = "protocol_stats",
    srcs = ["protocol_stats.cc"],
    data = glob(["testdata/protocol_stats*"]),
    deps = _DEPS_ + ["//dni/tasks/snding:protocol_stats_task"],
)

##################

cc_binary(
    name = "sip_base_merge",
    srcs = ["sip_base_merge.cc"],
    data = glob(["testdata/sip_base_merge*"]),
    deps = _DEPS_ + ["//dni/tasks/snding:sip_base_merge_task"],
)

##################

cc_binary(
    name = "sip_base_merge_dedup",
    srcs = ["sip_base_merge_dedup.cc"],
    data = glob(["testdata/sip_base_merge_dedup*"]),
    deps = _DEPS_ + ["//dni/tasks/snding:sip_base_merge_dedup_task"],
)

##################

cc_binary(
    name = "net_record_merge",
    srcs = ["net_record_merge.cc"],
    data = glob(["testdata/net_record_merge*"]),
    deps = _DEPS_ + ["//dni/tasks/snding:net_record_merge_task"],
)

##################

cc_binary(
    name = "dms_rules_test",
    srcs = ["dms_rules_test.cc"],
    data = glob(["testdata/dms_rules_test*"]),
    deps = _DEPS_ + ["//dni/tasks/snding:snding_defines"],
)

#####

cc_binary(
    name = "dms_rules_1",
    srcs = ["dms_rules_1.cc"],
    data = glob(["testdata/dms_rules_1*"]),
    deps = _DEPS_ + [
        "//dni/tasks/snding:dms_rules_task",
        "//dni/tasks/snding:sip_base_merge_task",
    ],
)

cc_binary(
    name = "dms_rules_2",
    srcs = ["dms_rules_2.cc"],
    data = glob(["testdata/dms_rules_2*"]),
    deps = _DEPS_ + [
        "//dni/tasks/snding:dms_rules_task",
        "//dni/tasks/snding:sip_base_merge_task",
    ],
)

cc_binary(
    name = "dms_rules_3",
    srcs = ["dms_rules_3.cc"],
    data = glob(["testdata/dms_rules_3*"]),
    deps = _DEPS_ + [
        "//dni/tasks/snding:dms_rules_task",
        "//dni/tasks/snding:sip_base_merge_task",
    ],
)

##################

cc_binary(
    name = "dms_rules_dedup_1",
    srcs = ["dms_rules_dedup_1.cc"],
    data = glob(["testdata/dms_rules_dedup_1*"]),
    deps = _DEPS_ + [
        "//dni/tasks/snding:dms_rules_dedup_task",
        "//dni/tasks/snding:sip_base_merge_dedup_task",
    ],
)

cc_binary(
    name = "dms_rules_dedup_2",
    srcs = ["dms_rules_dedup_2.cc"],
    data = glob(["testdata/dms_rules_dedup_2*"]),
    deps = _DEPS_ + [
        "//dni/tasks/snding:dms_rules_dedup_task",
        "//dni/tasks/snding:sip_base_merge_dedup_task",
    ],
)

cc_binary(
    name = "dms_rules_dedup_3",
    srcs = ["dms_rules_dedup_3.cc"],
    data = glob(["testdata/dms_rules_dedup_3*"]),
    deps = _DEPS_ + [
        "//dni/tasks/snding:dms_rules_dedup_task",
        "//dni/tasks/snding:sip_base_merge_dedup_task",
    ],
)

cc_binary(
    name = "dms_rules_dedup_4",
    srcs = ["dms_rules_dedup_4.cc"],
    data = glob(["testdata/dms_rules_dedup_4*"]),
    deps = _DEPS_ + [
        "//dni/tasks/snding:dms_rules_dedup_task",
        "//dni/tasks/snding:sip_base_merge_dedup_task",
    ],
)
