# -*- coding: utf-8 -*-
# @Author: Jelly
# @Email: jellyHello@163.com
# @Time: 2024-03-20 10:55:26


class AbnormalResourceLatest:
    def __init__(self, resource_stat, abnormal_nic):
        self.resource_stat = resource_stat
        self.abnormal_nic = abnormal_nic

    def abnormal_resource_acquire(self):
        abnormal_host = list(set([temp.split("#")[0] for temp in self.abnormal_nic]))
        abnormal_resource_latest = self.resource_stat[
            self.resource_stat["hostSign"].isin(abnormal_host)]
        node_output = {
            "abnormal_resource_latest": abnormal_resource_latest
        }
        return node_output


if __name__ == '__main__':
    pbtxt_path = "conf/scene/ddos_detection/ddos_anomaly_detection_b1m2.pbtxt"

    from scripts.graph.graph import Graph
    G = Graph(pbtxt_path)
    graph_input = G.graph_input_manage()
    resource_stat = graph_input["resource_stat"]
    # scheduler executor telemetry_abnormal_nic
    # abnormal_nic = ['asn52-614#eno3', 'asn96-614#eno1']
    abnormal_nic = delayed_res["abnormal_nic"]

    resource_stat = graph_outputs["resource_stat"]
    abnormal_nic = graph_outputs["abnormal_nic"]

    ARL = AbnormalResourceLatest(resource_stat, abnormal_nic)
    node_output = ARL.abnormal_resource_acquire()
