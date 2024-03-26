# -*- coding: utf-8 -*-
# @Author: Jelly
# @Email: jellyHello@163.com
# @Time: 2024-03-22 15:19:39
import pandas as pd

from conf.system.param_parse import param_parse


class PacketStatStatus:
    def __init__(self, packet_latest):
        self.packet_latest = packet_latest

    def analysis_packet_latest(self):
        config = param_parse()
        normalThreshold = int(config.get('base', 'normal_threshold'))
        selStatDF = self.packet_latest[self.packet_latest["countTotal"] >= normalThreshold]
        return selStatDF

    def assess_packet_status(self):
        abnormal_basis = self.analysis_packet_latest()
        abnormal_nic = sorted(list(set(abnormal_basis["hostNicSign"])))
        normal_nic = sorted(list(set(self.packet_latest["hostNicSign"]) - set(abnormal_nic)))
        normal_a1m2_output = {
            "abnormal_nic": abnormal_nic,
            "normal_nic": normal_nic
        }
        node_output = {
            "normal_a1m2_output": normal_a1m2_output,
            "normal_a1m2_basis": abnormal_basis
        }
        return node_output


if __name__ == '__main__':
    pbtxt_path = "conf/scene/ddos_detection/ddos_normal_detection_a1m2.pbtxt"

    from scripts.graph.graph import Graph
    G = Graph(pbtxt_path)
    input_stream_dict = G.graph_input_manager()
    packet_initial = input_stream_dict["packet"]
    from scripts.executor.telemetry.telemetry_packet_stat_latest import PacketStatLatest
    PSL = PacketStatLatest(packet_initial)
    node_output = PSL.get_packet_stat()

    # start
    PSS = PacketStatStatus(node_output)
    node_output1 = PSS.assess_packet_status()
