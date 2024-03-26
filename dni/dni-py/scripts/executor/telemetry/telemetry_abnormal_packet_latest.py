# -*- coding: utf-8 -*-
# @Author: Jelly
# @Email: jellyHello@163.com
# @Time: 2024-03-20 10:55:26


class AbnormalPacketLatest:
    def __init__(self, packet_stat, abnormal_nic):
        self.packet_stat = packet_stat
        self.abnormal_nic = abnormal_nic

    def abnormal_packet_acquire(self):
        abnormal_packet_latest = self.packet_stat[
            self.packet_stat["hostNicSign"].isin(self.abnormal_nic)]
        node_output = {
            "abnormal_packet_latest": abnormal_packet_latest
        }
        return node_output


if __name__ == '__main__':
    pbtxt_path = "conf/scene/ddos_detection/ddos_anomaly_detection_b1m2.pbtxt"

    from scripts.graph.graph import Graph
    G = Graph(pbtxt_path)
    graph_input = G.graph_input_manage()
    packet_stat = graph_input["packet_stat"]
    # scheduler executor telemetry_abnormal_nic
    # abnormal_nic = ['asn52-614#eno3', 'asn96-614#eno1']
    abnormal_nic = delayed_res["abnormal_nic"]

    # packet_stat = node_inputs[0]
    # abnormal_nic = node_inputs[1]

    APL = AbnormalPacketLatest(packet_stat, abnormal_nic)
    node_output = APL.abnormal_packet_acquire()
