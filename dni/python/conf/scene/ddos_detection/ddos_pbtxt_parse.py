# -*- coding: utf-8 -*-
# @Author: Jelly
# @Email: jellyHello@163.com
# @Time: 2024-03-13 10:47:48

import google.protobuf.text_format as text_format
from conf.scene.ddos_detection.ddos_detection_config_pb2 import DDoSDetectionConfig


def parse_pbtxt(pbtxt_path):
    config = DDoSDetectionConfig()
    with open(pbtxt_path, "r") as f:
        text_format.Parse(f.read(), config)
    return config


if __name__ == "__main__":
    pbtxt_path = "conf/scene/ddos_detection/ddos_normal_detection_a1m2.pbtxt"
    config = parse_pbtxt(pbtxt_path)
    print(config)
