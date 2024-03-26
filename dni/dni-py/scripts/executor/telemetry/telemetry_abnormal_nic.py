# -*- coding: utf-8 -*-
# @Author: Jelly
# @Email: jellyHello@163.com
# @Time: 2024-03-23 14:19:42


class AbnormalNic:
    def __init__(self, normal_a1m2_basis):
        self.normal_a1m2_basis = normal_a1m2_basis

    def determine_abnormal_nic(self):
        abnormal_nic = sorted(list(set(self.normal_a1m2_basis["hostNicSign"])))
        node_output = {
            "abnormal_nic": abnormal_nic
        }
        return node_output


if __name__ == '__main__':

    from scripts.executor.graph.ddos_normal_detection_a1m2 import DDoSNormalDetectiona1m2
    pbtxt_path_a1m2 = "conf/scene/ddos_detection/ddos_normal_detection_a1m2.pbtxt"
    DND_a1m2 = DDoSNormalDetectiona1m2(pbtxt_path_a1m2)
    graph_output = DND_a1m2.get_a1m2_result()
    normal_a1m2_basis = graph_output["normal_a1m2_basis"]
    # start
    AN = AbnormalNic(normal_a1m2_basis)
    abnormal_nic = AN.determine_abnormal_nic()
