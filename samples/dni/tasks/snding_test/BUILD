

load("//samples:variables.bzl", "DEPS")
load("@rules_cc//cc:defs.bzl", "cc_library", "cc_binary")
_DEPS_ = DEPS + [
    "//dni/framework",
    "@spdlog",
]
_SRCS_ = [
    "testenv.h",
]

cc_binary(
    name = "threshold",
    srcs =   _SRCS_ + ["threshold.cc"],
    data = glob(["testdata/threshold*"]),
    deps = _DEPS_ + [
        "//dni/tasks/core:threshold_task",
    ],
)

############

cc_binary(
    name = "sum",
    srcs =   _SRCS_ + ["sum.cc"],
    data = glob(["testdata/sum*"]),
    deps = _DEPS_ + [
        "//dni/tasks/core:sum_task",
    ],
)

############

cc_binary(
    name = "anomaly-detection",
    srcs =   _SRCS_ + ["anomaly-detection.cc"],
    data = glob(["testdata/anomaly_detection*"]),
    deps = _DEPS_ + [
        "//dni/tasks/snding:anomaly_detection_task",
    ],
)

############

cc_binary(
    name = "counter",
    srcs =   _SRCS_ + ["counter.cc"],
    data = glob(["testdata/counter*"]),
    deps = _DEPS_ + [
        "//dni/tasks/core:counter_task",
    ],
)

############

cc_binary(
    name = "cond-threshold",
    srcs =  _SRCS_ + ["cond-threshold.cc"],
    data = glob(["testdata/cond_threshold*"]),
    deps = _DEPS_ + [
        "//dni/tasks/core:cond_threshold_task",
    ],
)

############

cc_binary(
    name = "number-stats",
    srcs =  _SRCS_ + ["number-stats.cc"],
    data = glob(["testdata/number_stats*"]),
    deps = _DEPS_ + [
        "//dni/tasks/snding:feature_counter_task",
        "//dni/tasks/snding:pcap_parse_task",
        "//dni/tasks/snding:number_stats_task",
    ],
)

############

cc_binary(
    name = "protocol-stats",
    srcs = _SRCS_ + [
        "protocol-stats.cc",
    ],
    data = glob(["testdata/protocol_stats*"]),
    deps = _DEPS_ + [
        "//dni/tasks/snding:feature_counter_task",
        "//dni/tasks/snding:pcap_parse_task",
        "//dni/tasks/snding:number_stats_task",
        "//dni/tasks/snding:protocol_stats_task",
    ],
)

############

cc_binary(
    name = "max-number",
    srcs = _SRCS_ + [
        "max-number.cc",
    ],
    data = glob(["testdata/max_number*"]),
    deps = _DEPS_ + [
        "//dni/tasks/snding:feature_counter_task",
        "//dni/tasks/snding:pcap_parse_task",
        "//dni/tasks/snding:number_stats_task",
        "//dni/tasks/snding:protocol_stats_task",
        "//dni/tasks/core:max_task",
    ],
)

##################

cc_binary(
    name = "pcap-parse",
    srcs = _SRCS_ + ["pcap-parse.cc"],
    data = glob(["testdata/pcap_parse*"]),
    deps = _DEPS_ + [
        "//dni/tasks/snding:pcap_parse_task",
    ],
)

##################

cc_binary(
    name = "feature-counter",
    srcs = _SRCS_ + [
        "feature-counter.cc",
    ],
    data = glob(["testdata/feature_counter*"]),
    deps = _DEPS_ + [
        "//dni/tasks/snding:feature_counter_task",
        "//dni/tasks/snding:pcap_parse_task",
    ],
)

##################

cc_binary(
    name = "attacker-ip-merge",
    srcs =  _SRCS_ + ["attacker-ip-merge.cc"],
    data = glob(["testdata/attacker_ip_merge*"]),
    deps = _DEPS_ + [
        "//dni/tasks/snding:feature_counter_task",
        "//dni/tasks/snding:pcap_parse_task",
        "//dni/tasks/snding:number_stats_task",
        "//dni/tasks/snding:protocol_stats_task",
        "//dni/tasks/core:max_task",
        "//dni/tasks/snding:attacker_ip_merge_task",
    ],
)

##################

cc_binary(
    name = "sip-base-merge",
    srcs =  _SRCS_ + [
        "sip-base-merge.cc",
    ],
    data = glob(["testdata/sip_base_merge*"]),
    deps = _DEPS_ + [
        "//dni/tasks/snding:feature_counter_task",
        "//dni/tasks/snding:pcap_parse_task",
        "//dni/tasks/snding:number_stats_task",
        "//dni/tasks/snding:protocol_stats_task",
        "//dni/tasks/core:max_task",
        "//dni/tasks/snding:attacker_ip_merge_task",
        "//dni/tasks/snding:sip_base_merge_task",
    ],
)

##################
cc_binary(
    name = "sip-base-merge-dedup",
    srcs =  _SRCS_ + [
        "sip-base-merge-dedup.cc",
    ],
    data = glob(["testdata/sip_base_merge_dedup*"]),
    deps = _DEPS_ + [
        "//dni/tasks/snding:feature_counter_task",
        "//dni/tasks/snding:pcap_parse_task",
        "//dni/tasks/snding:number_stats_task",
        "//dni/tasks/snding:protocol_stats_task",
        "//dni/tasks/core:max_task",
        "//dni/tasks/snding:attacker_ip_merge_task",
        "//dni/tasks/snding:sip_base_merge_dedup_task",
    ],
)

##################
cc_binary(
    name = "net-record-merge",
    srcs =  _SRCS_ + [
        "net-record-merge.cc",
    ],
    data = glob(["testdata/net_record_merge*"]),
    deps = _DEPS_ + [
        "//dni/tasks/snding:feature_counter_task",
        "//dni/tasks/snding:pcap_parse_task",
        "//dni/tasks/snding:number_stats_task",
        "//dni/tasks/snding:protocol_stats_task",
        "//dni/tasks/core:max_task",
        "//dni/tasks/snding:attacker_ip_merge_task",
        "//dni/tasks/snding:sip_base_merge_dedup_task",
        "//dni/tasks/snding:net_record_merge_task",
    ],
)

##################
cc_binary(
    name = "dms-rule-dedup",
    srcs =  _SRCS_ + [
        "dms-rule-dedup.cc",
    ],
    data = glob(["testdata/net_record_merge*",
    "testdata/dms_rules_dedup*",]),
    deps = _DEPS_ + [
        "//dni/tasks/snding:feature_counter_task",
        "//dni/tasks/snding:pcap_parse_task",
        "//dni/tasks/snding:number_stats_task",
        "//dni/tasks/snding:protocol_stats_task",
        "//dni/tasks/core:max_task",
        "//dni/tasks/snding:attacker_ip_merge_task",
        "//dni/tasks/snding:sip_base_merge_dedup_task",
        "//dni/tasks/snding:net_record_merge_task",
        "//dni/tasks/snding:dms_rules_dedup_task",
    ],
)

##################
cc_binary(
    name = "test",
    srcs =  _SRCS_ + [
        "test.cc",
    ],
    data = glob(["testdata/*"]),
    deps = _DEPS_ + [
        "//dni/tasks/core:cond_threshold_task",
        "//dni/tasks/core:max_task",
        "//dni/tasks/core:sum_task",
        "//dni/tasks/core:threshold_task",
        "//dni/tasks/snding:anomaly_detection_task",
        "//dni/tasks/snding:attacker_ip_merge_task",
        "//dni/tasks/snding:dms_rules_dedup_task",
        "//dni/tasks/snding:feature_counter_task",
        "//dni/tasks/snding:net_record_merge_task",
        "//dni/tasks/snding:number_stats_task",
        "//dni/tasks/snding:pcap_parse_task",
        "//dni/tasks/snding:protocol_stats_task",
        "//dni/tasks/snding:sip_base_merge_dedup_task",
    ],
)