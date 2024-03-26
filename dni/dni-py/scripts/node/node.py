# -*- coding: utf-8 -*-
# @Author: Jelly
# @Email: jellyHello@163.com
# @Time: 2024-03-15 15:46:54

from scripts.executor.executor import Execute


class Node:
    def __init__(self, node, graph_outputs):
        self.node = node
        self.graph_outputs = graph_outputs

    def node_execute(self):
        executor_name = self.node["executor_name"]
        input_stream_node = list(self.node["input_stream_node"])
        node_inputs = []
        for input_stream in input_stream_node:
            # input_stream = input_stream_node[0]
            graph_input_list = list(self.graph_outputs.keys())
            if input_stream in graph_input_list:
                node_input = self.graph_outputs[input_stream]
                node_inputs.append(node_input)
            else:
                print("# failed to obtian: ", input_stream)
        E = Execute(executor_name, node_inputs)
        node_output = E.execute_output()
        return node_output


if __name__ == "__main__":
    pbtxt_path = "conf/scene/ddos_detection/ddos_normal_detection_a1m2.pbtxt"
    from scripts.graph.graph import Graph
    G = Graph(pbtxt_path)
    graph_input = G.graph_input_manage()
    nodes = G.graph_nodes_acquire()
    node = nodes[nodes["node_name"] == "node0"]


    N = Node(node, graph_input)
    node_output = N.node_execute()

