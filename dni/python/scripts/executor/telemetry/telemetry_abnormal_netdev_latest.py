# -*- coding: utf-8 -*-
# @Author: Jelly
# @Email: jellyHello@163.com
# @Time: 2024-03-20 10:55:26


class AbnormalNetdevLatest:
    def __init__(self, netdev_stat, abnormal_nic):
        self.netdev_stat = netdev_stat
        self.abnormal_nic = abnormal_nic

    def abnormal_netdev_acquire(self):
        abnormal_netdev_latest = self.netdev_stat[
            self.netdev_stat["hostNicSign"].isin(self.abnormal_nic)]
        node_output = {
            "abnormal_netdev_latest": abnormal_netdev_latest
        }
        return node_output


if __name__ == '__main__':
    pbtxt_path = "conf/scene/ddos_detection/ddos_anomaly_detection_b1m2.pbtxt"

    from scripts.graph.graph import Graph
    G = Graph(pbtxt_path)
    graph_input = G.graph_input_manage()
    netdev_stat = graph_input["netdev_stat"]
    # scheduler executor telemetry_abnormal_nic
    # abnormal_nic = ['asn52-614#eno3', 'asn96-614#eno1']
    abnormal_nic = delayed_res["abnormal_nic"]

    ANL = AbnormalNetdevLatest(netdev_stat, abnormal_nic)
    node_output = ANL.abnormal_netdev_acquire()
