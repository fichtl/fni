# -*- coding: utf-8 -*-
# @Author: Jelly
# @Email: jellyHello@163.com
# @Time: 2024-03-13 19:12:02

from scripts.graph.graph_parse import GraphParse


class GraphManage:
    def __init__(self, pbtxt_path):
        self.pbtxt_path = pbtxt_path

    def graph_initialize(self):
        return

    def graph_config(self):
        GP = GraphParse(self.pbtxt_path)
        input_stream, output_stream, nodes = GP.graph_parse()
        return input_stream, output_stream, nodes

    def graph_self_executor(self):
        return

    def graph_run_once(self):
        return

    def graph_run(self):
        return

    def graph_wait(self):
        return

    def graph_wait_observed_output(self):
        return

    def graph_has_error(self):
        return

    def graph_packet_input_stream(self):
        return

    def graph_output_stream_callback(self):
        return

    def graph_output_packet(self):
        return

    def graph_close_input_stream(self):
        return

    def graph_pause(self):
        return

    def graph_resume(self):
        return

    def graph_cancel(self):
        return


if __name__ == "__main__":
    pbtxt_path = "conf/scene/ddos_detection/ddos_detection_demo.pbtxt"
    GM = GraphManage(pbtxt_path)

