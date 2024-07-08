# -*- coding: utf-8 -*-
# @Author: Jelly
# @Email: jellyHello@163.com
# @Time: 2024-03-14 09:28:23

from scripts.graph.graph_manage import GraphManage
from scripts.graph.input_transform import InputTransform
from scripts.scheduler.scheduler import Scheduler


class Graph:
    def __init__(self, pbtxt_path):
        self.pbtxt_path = pbtxt_path
        GM = GraphManage(self.pbtxt_path)
        self.input_stream, self.output_stream, self.nodes = GM.graph_config()

    def graph_input_acquire(self):
        IT = InputTransform(self.input_stream)
        input_stream_dict = IT.input_transform()
        return input_stream_dict

    def graph_input_manage(self):
        input_stream_dict = self.graph_input_acquire()
        graph_inputs = input_stream_dict.copy()
        return graph_inputs

    def graph_nodes_acquire(self):
        graph_nodes = self.nodes
        return graph_nodes

    def graph_schedule(self):
        graph_inputs = self.graph_input_manage()
        graph_nodes = self.graph_nodes_acquire()
        S = Scheduler(graph_nodes, graph_inputs)
        graph_outputs = S.nodes_schedule()
        return graph_outputs

    def graph_output(self):
        graph_outputs = self.graph_schedule()
        graph_output = {}
        for output_name in self.output_stream:
            graph_output[output_name] = graph_outputs[output_name]
        return graph_output

    def graph_output_manage(self):
        graph_output = self.graph_output()
        return graph_output

    def graph_manage(self):
        graph_output = self.graph_output_manage()
        return graph_output


if __name__ == "__main__":

    import time
    start_time = time.time()

    pbtxt_path = "conf/scene/ddos_detection/ddos_normal_detection_a1m2.pbtxt"
    # pbtxt_path = "conf/scene/ddos_detection/ddos_anomaly_detection_b1m2.pbtxt"
    G = Graph(pbtxt_path)
    graph_output = G.graph_manage()

    end_time = time.time()
    runtime = round(end_time - start_time, 4)
    print("runtime: %s s" % runtime)

    for key, value in graph_output.items():
        print(key)
        print(value)
