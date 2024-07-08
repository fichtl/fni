# -*- coding: utf-8 -*-
# @Author: Jelly
# @Email: jellyHello@163.com
# @Time: 2024-03-14 17:19:16

import os
import sys
import pandas as pd


def graph_taskflow(pbtxt_path):
    from scripts.graph.graph import Graph
    G = Graph(pbtxt_path)
    graph_output = G.graph_manage()
    return graph_output


class DDoSDetection:
    def normal_detection(self, pbtxt_path_a1m2):
        graph_output = graph_taskflow(pbtxt_path_a1m2)
        normal_a1m2_output = graph_output["normal_a1m2_output"]
        normal_a1m2_basis = graph_output["normal_a1m2_basis"]
        return normal_a1m2_output, normal_a1m2_basis

    def anomaly_detection(self, pbtxt_path_b1m2):
        graph_output = graph_taskflow(pbtxt_path_b1m2)
        b1m2_stat_preprocess = graph_output["b1m2_stat_preprocess"]
        return b1m2_stat_preprocess


def main():
    # sys path
    project_path = os.getcwd()
    sys.path.append(project_path)

    # data display setting
    # import pandas as pd
    pd.set_option('display.expand_frame_repr', False)
    pd.set_option('display.max_colwidth', None)

    DDD = DDoSDetection()

    # normal_detection
    pbtxt_path_a1m2 = "conf/scene/ddos_detection/ddos_normal_detection_a1m2.pbtxt"
    normal_a1m2_output, normal_a1m2_basis = DDD.normal_detection(pbtxt_path_a1m2)
    print("normal_a1m2_output:\n", normal_a1m2_output)

    # anomaly_detection
    pbtxt_path_b1m2 = "conf/scene/ddos_detection/ddos_anomaly_detection_b1m2.pbtxt"
    b1m2_stat_preprocess = DDD.anomaly_detection(pbtxt_path_b1m2)
    print("b1m2_stat_preprocess:\n", b1m2_stat_preprocess)


if __name__ == "__main__":
    main()

