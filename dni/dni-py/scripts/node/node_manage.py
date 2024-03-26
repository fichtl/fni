# -*- coding: utf-8 -*-
# @Author: Jelly
# @Email: jellyHello@163.com
# @Time: 2024-03-13 17:49:16

from scripts.node.node import Node


class NodeManage:
    def __init__(self, node, graph_outputs):
        self.node = node
        self.graph_outputs = graph_outputs

    def node_initialize(self):
        return

    def node_prepare_run(self):
        return

    def node_set_executor(self):
        return

    def node_ready(self):
        return

    def node_process_task(self):
        return

    def node_start_task(self):
        N = Node(self.node, self.graph_outputs)
        node_output = N.node_execute()
        return node_output

    def node_stop_task(self):
        return

    def node_context(self):
        return

    def node_state(self):
        return


if __name__ == "__main__":
    pbtxt_path = "conf/scene/ddos_detection/ddos_normal_detection_a1m2.pbtxt"
    from scripts.graph.graph import Graph
    G = Graph(pbtxt_path)
    graph_input = G.graph_input_manage()
    nodes = G.graph_nodes_acquire()
    node = nodes[nodes["node_name"] == "node0"]

    NM = NodeManage(node, graph_outputs)
    node_output = NM.node_start_task()


