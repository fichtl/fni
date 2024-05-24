cc_library(
    name = "influxdb",
    hdrs = ["influxdb.hpp"],
    visibility = ["//visibility:public"],
)

cc_binary(
    name = "getData",
    srcs = ["getData.cc"],
    data = [
        "pbtxt/graph1.pbtxt",
        "pbtxt/graph2.pbtxt",
        "pbtxt/graph3.pbtxt",
    ],
    deps = [
        ":influxdb",
        "//dni/framework",
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
        "@fmtlib",
        "@jsoncpp",
        "@spdlog",
    ],
)

cc_binary(
    name = "getData_countTotal",
    srcs = ["getData_countTotal.cc"],
    deps = [
        ":influxdb",
        "@jsoncpp",
    ],
)

cc_binary(
    name = "getData_packet",
    srcs = ["getData_packet.cc"],
    deps = [
        ":influxdb",
        "@jsoncpp",
    ],
)

cc_binary(
    name = "getData_stats",
    srcs = ["getData_stats.cc"],
    deps = [
        ":influxdb",
        "@jsoncpp",
    ],
)

cc_binary(
    name = "realtime_anomaly_detect",
    srcs = ["realtime_anomaly_detect.cc"],
    data = ["pbtxt/graph1.pbtxt"],
    deps = [
        ":influxdb",
        "//dni/framework",
        "//dni/tasks/core:cond_threshold_task",
        "//dni/tasks/core:sum_task",
        "//dni/tasks/core:threshold_task",
        "//dni/tasks/snding:anomaly_detection_task",
        "@fmtlib",
        "@jsoncpp",
        "@spdlog",
    ],
)

cc_binary(
    name = "realtime_attack_detect",
    srcs = ["realtime_attack_detect.cc"],
    data = [
        "pbtxt/graph1.pbtxt",
        "pbtxt/graph2.pbtxt",
    ],
    deps = [
        ":influxdb",
        "//dni/framework",
        "//dni/tasks/core:cond_threshold_task",
        "//dni/tasks/core:max_task",
        "//dni/tasks/core:sum_task",
        "//dni/tasks/core:threshold_task",
        "//dni/tasks/snding:anomaly_detection_task",
        "//dni/tasks/snding:feature_counter_task",
        "//dni/tasks/snding:number_stats_task",
        "//dni/tasks/snding:pcap_parse_task",
        "//dni/tasks/snding:protocol_stats_task",
        "@fmtlib",
        "@jsoncpp",
        "@spdlog",
    ],
)

cc_binary(
    name = "single_detect_defense",
    srcs = ["single_detect_defense.cc"],
    data = [
        "pbtxt/graph1.pbtxt",
        "pbtxt/graph2.pbtxt",
        "pbtxt/graph3.pbtxt",
    ],
    deps = [
        ":influxdb",
        "//dni/framework",
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
        "@fmtlib",
        "@jsoncpp",
        "@spdlog",
    ],
)
