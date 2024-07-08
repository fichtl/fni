# -*- coding: utf-8 -*-
# @Author: Jelly
# @Email: jellyHello@163.com
# @Time: 2024-03-13 15:58:33

import re
import pandas as pd
from conf.scene.ddos_detection.ddos_pbtxt_parse import parse_pbtxt


class GraphParse:
    def __init__(self, pbtxt_path):
        self.pbtxt_path = pbtxt_path

    def graph_config(self):
        config = parse_pbtxt(self.pbtxt_path)
        return config

    def graph_parse(self):
        config = self.graph_config()
        # 1) parse input_stream
        input_stream_str = str(config.input_stream).replace("\n", ", ")
        input_stream = re.findall(r'"([^"]*)"', input_stream_str)
        # 2) parse output_stream
        output_stream_str = str(config.output_stream).replace("\n", ", ")
        output_stream = re.findall(r'"([^"]*)"', output_stream_str)
        # 3) parse nodes
        nodes_list = config.node
        nodesL = []
        for i, node in enumerate(nodes_list):
            # i = 0
            # nodes_ = nodes_list[0]
            node_name = "".join(["node", str(i)])
            executor_name = re.findall(r'executor:\s*"([^"]+)"', str(node))[0]
            input_stream_node = re.findall(r'input_stream:\s*"([^"]+)"', str(node))
            output_stream_node = re.findall(r'output_stream:\s*"([^"]+)"', str(node))
            node_list = [node_name, executor_name,
                         input_stream_node, output_stream_node]
            nodesL.append(node_list)
        nodes = pd.DataFrame(nodesL, columns=["node_name", "executor_name",
                                              "input_stream_node", "output_stream_node"
                                              ])
        return input_stream, output_stream, nodes


if __name__ == "__main__":
    pbtxt_path = "conf/scene/ddos_detection/ddos_anomaly_detection_b1m2.pbtxt"

    GP = GraphParse(pbtxt_path)
    input_stream, output_stream, nodes = GP.graph_parse()
    print("input_stream: ", input_stream)
    print("output_stream: ", output_stream)

    # data display setting
    import pandas as pd
    pd.set_option('display.expand_frame_repr', False)
    pd.set_option('display.max_colwidth', None)

    print("nodes: \n", nodes)


