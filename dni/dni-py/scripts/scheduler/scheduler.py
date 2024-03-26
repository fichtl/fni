# -*- coding: utf-8 -*-
# @Author: Jelly
# @Email: jellyHello@163.com
# @Time: 2024-03-13 17:55:42

from dask import delayed
from scripts.node.node_manage import NodeManage


class Scheduler:
    def __init__(self, nodes, graph_input):
        self.nodes = nodes
        self.graph_input = graph_input

    def nodes_relate(self):
        graph_outputs = self.graph_input.copy()
        for idx in range(len(self.nodes)):
            # idx = 0
            node = self.nodes.iloc[idx]
            NM = NodeManage(node, graph_outputs)
            node_output = NM.node_start_task()
            graph_outputs = {**graph_outputs, **node_output}
        return graph_outputs

    def nodes_schedule(self):
        graph_outputs = self.nodes_relate()
        return graph_outputs


if __name__ == "__main__":
    # premise
    pbtxt_path = "conf/scene/ddos_detection/ddos_normal_detection_a1m2.pbtxt"
    # pbtxt_path = "conf/scene/ddos_detection/ddos_anomaly_detection_b1m2.pbtxt"

    from scripts.graph.graph import Graph
    G = Graph(pbtxt_path)
    graph_input = G.graph_input_manage()
    nodes = G.graph_nodes_acquire()

    # start
    S = Scheduler(nodes, graph_input)
    graph_outputs = S.nodes_schedule()

    for key, value in graph_outputs.items():
        print(key)
        print(value)


