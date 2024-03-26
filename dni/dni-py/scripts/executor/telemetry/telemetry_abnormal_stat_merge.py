#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# author: Jelly
# E-mail: jellyHello@163.com
# time: 2024-03-24 22:53:28

import pandas as pd


class AbnormalStatMerge:
    def __init__(self, abnormal_packet_latest,
                 abnormal_netdev_latest,
                 abnormal_resource_latest):

        self.abnormal_packet_latest = abnormal_packet_latest
        self.abnormal_netdev_latest = abnormal_netdev_latest
        self.abnormal_resource_latest = abnormal_resource_latest

    def abnormal_stat_merge(self):
        # merge packet and netdev data
        mergeCol1 = list(set(self.abnormal_packet_latest.columns) & set(self.abnormal_netdev_latest.columns))
        # print("mergeCol1:", mergeCol1)
        packet_netdev_latestDF = pd.merge(self.abnormal_packet_latest, self.abnormal_netdev_latest,
                                          on=mergeCol1)
        # print("packet_netdev_latestDF['hostNicSign']:", sorted(set(packet_netdev_latestDF['hostNicSign'])))
        # merge packet&netdev and resource data
        mergeCol2 = list(set(packet_netdev_latestDF.columns) & set(self.abnormal_resource_latest.columns))
        # print("mergeCol2:", mergeCol2)
        abnormal_stat_latest = pd.merge(packet_netdev_latestDF, self.abnormal_resource_latest, on=mergeCol2)
        node_output = {
            "abnormal_stat_latest": abnormal_stat_latest
        }
        return node_output


if __name__ == '__main__':
    pbtxt_path = "conf/scene/ddos_detection/ddos_anomaly_detection_b1m2.pbtxt"
    from scripts.graph.graph import Graph
    G = Graph(pbtxt_path)
    input_stream_dict = G.graph_input_manage()

    # start
    abnormal_packet_latest = pd.DataFrame()
    abnormal_netdev_latest = pd.DataFrame()
    abnormal_resource_latest = pd.DataFrame()
    ASM = AbnormalStatMerge(abnormal_packet_latest,
                 abnormal_netdev_latest,
                 abnormal_resource_latest)