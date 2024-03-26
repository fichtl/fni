# -*- coding: utf-8 -*-
# @Author: Jelly
# @Email: jellyHello@163.com
# @Time: 2024-03-18 15:56:40


class InputTransform:
    def __init__(self, input_stream):
        self.input_stream = input_stream

    def graph_input_reference(self):
        graph_input_reference = ["packet_initial", "packet_stat", "netdev_stat", "resource_stat",
                                 "pcap", "normal_a1m2_basis"]
        return graph_input_reference

    def packet_initial_transform(self):
        from api.influxdb.packet_initial import PacketInitial
        PI = PacketInitial()
        packet_initial = PI.get_packet_initial()
        return packet_initial

    def packet_stat_transform(self):
        from api.influxdb.packet_stat import PacketStat
        PS = PacketStat()
        packet_stat = PS.get_packet_data()
        return packet_stat

    def netdev_stat_transform(self):
        from api.influxdb.netdev_stat import NetdevStat
        NS = NetdevStat()
        netdev_stat = NS.get_netdev_data()
        return netdev_stat

    def resource_stat_transform(self):
        from api.influxdb.resource_stat import ResourceStat
        RS = ResourceStat()
        resource_stat = RS.get_resource_data()
        return resource_stat

    def normal_a1m2_basis_transform(self, pbtxt_path_a1m2):
        from scripts.executor.graph.ddos_normal_detection_a1m2 import DDoSNormalDetectiona1m2
        DND_a1m2 = DDoSNormalDetectiona1m2(pbtxt_path_a1m2)
        normal_a1m2_basis = DND_a1m2.get_a1m2_result()
        return normal_a1m2_basis

    def pcap_transform(self):
        pcap_initial = []
        return pcap_initial

    def input_transform(self):
        input_stream_dict = {}
        graph_input_reference = self.graph_input_reference()
        for input_ in self.input_stream:
            # input_ = input_stream[0]
            if input_ in graph_input_reference:
                # input_stream: packet
                if input_ == "packet_initial":
                    packet_initial = self.packet_initial_transform()
                    input_stream_dict[input_] = packet_initial
                elif input_ == "packet_stat":
                    packet_stat = self.packet_stat_transform()
                    input_stream_dict[input_] = packet_stat
                elif input_ == "netdev_stat":
                    netdev_stat = self.netdev_stat_transform()
                    input_stream_dict[input_] = netdev_stat
                elif input_ == "resource_stat":
                    resource_stat = self.resource_stat_transform()
                    input_stream_dict[input_] = resource_stat
                elif input_ == "normal_a1m2_basis":
                    pbtxt_path_a1m2 = "conf/scene/ddos_detection/ddos_normal_detection_a1m2.pbtxt"
                    normal_a1m2_basis = self.normal_a1m2_basis_transform(pbtxt_path_a1m2)
                    input_stream_dict[input_] = normal_a1m2_basis[input_]
                elif input_ == "pcap":
                    pcap_initial = self.pcap_transform()
                    input_stream_dict[input_] = pcap_initial
                else:
                    print("# [error] graph_input_reference value matching error! (%s)" % input_)
            else:
                print("# [error] please enter a correct input_stream value! (%s)" % input_)
        return input_stream_dict


if __name__ == '__main__':
    pbtxt_path_b1m2 = "conf/scene/ddos_detection/ddos_anomaly_detection_b1m2.pbtxt"

    from scripts.graph.graph_manage import GraphManage
    GM = GraphManage(pbtxt_path_b1m2)
    input_stream, output_stream, nodes = GM.graph_config()

    IT = InputTransform(input_stream)
    input_stream_dict = IT.input_transform()
