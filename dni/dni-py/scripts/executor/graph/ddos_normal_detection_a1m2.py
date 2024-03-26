# -*- coding: utf-8 -*-
# @Author: Jelly
# @Email: jellyHello@163.com
# @Time: 2024-03-22 11:28:18

class DDoSNormalDetectiona1m2:
    def __init__(self, pbtxt_path_a1m2):
        self.pbtxt_path_a1m2 = pbtxt_path_a1m2

    def graph_taskflow(self):
        from scripts.graph.graph import Graph
        G = Graph(self.pbtxt_path_a1m2)
        graph_output = G.graph_manage()
        return graph_output

    def get_a1m2_result(self):
        graph_output = self.graph_taskflow()
        return graph_output


if __name__ == '__main__':
    pbtxt_path_a1m2 = "conf/scene/ddos_detection/ddos_normal_detection_a1m2.pbtxt"

    DND_a1m2 = DDoSNormalDetectiona1m2(pbtxt_path_a1m2)
    graph_output = DND_a1m2.get_a1m2_result()

