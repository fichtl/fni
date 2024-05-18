load("//samples:variables.bzl", "DEPS")

_DEPS_ = DEPS + [
    "//dni/framework",
    "@spdlog",
]

cc_binary(
    name = "attack_tracing",
    srcs = ["attack_tracing.cc"],
    data = glob(["testdata/attack_tracing*"]),
    deps = _DEPS_ + [
        "//dni/tasks/snding:attacker_ip_merge_task",
        "//dni/tasks/snding:feature_counter_task",
        "//dni/tasks/snding:pcap_parse_task",
    ],
)
