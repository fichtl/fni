# -*- coding: utf-8 -*-
# @Author: Jelly
# @Email: jellyHello@163.com
# @Time: 2024-03-18 17:13:33

import networkx as nx


class NodesRelate:
    def __init__(self, nodes, input_stream, output_stream):
        self.nodes = nodes
        self.input_stream = input_stream
        self.output_stream = output_stream

    def search_node_se(self):
        start_node_list = []
        end_node_list = []
        for idx in range(len(self.nodes)):
            # idx = 6
            # start_node
            input_stream_node = self.nodes.iloc[idx]["input_stream_node"]
            start_intersect_set = set(input_stream_node) & set(self.input_stream)
            if set(input_stream_node) == start_intersect_set:
                node_name = self.nodes.iloc[idx]["node_name"]
                start_node_list.append(node_name)
            else:
                pass
            # end_node
            output_stream_node = self.nodes.iloc[idx]["output_stream_node"]
            end_intersect_set = set(output_stream_node) & set(self.output_stream)
            if set(output_stream_node) == end_intersect_set:
                node_name = self.nodes.iloc[idx]["node_name"]
                end_node_list.append(node_name)
            else:
                pass
        return start_node_list, end_node_list

    def get_next_node(self, node_name):
        # node_name = source_node_list[0]
        output_stream_node = list(self.nodes[self.nodes["node_name"] ==
                                        node_name]["output_stream_node"])[0]
        input_stream_nodes = self.nodes["input_stream_node"]
        relate_path_list = []
        for output_ in output_stream_node:
            # output_ = output_stream_node[0]
            for idx, input_l in enumerate(input_stream_nodes):
                # idx = 3
                # input_l = input_stream_nodes[idx]
                if output_ in input_l:
                    node_name_next = self.nodes.iloc[idx]["node_name"]
                    relate_path = "%s-->%s" % (node_name, node_name_next)
                    relate_path_list.append(relate_path)
        return relate_path_list

    def find_all_paths(self, graph, start_node, end_node):
        all_paths = []
        for path in nx.all_simple_paths(graph, source=start_node, target=end_node):
            all_paths.append(path)
        return all_paths

    def find_all_paths_from_start(self, graph, start_node):
        all_paths = []
        def dfs_paths(node, path):
            path.append(node)
            if not graph.out_edges(node):
                all_paths.append(list(path))
            else:
                for neighbor in graph.neighbors(node):
                    dfs_paths(neighbor, path)
            path.pop()
        dfs_paths(start_node, [])
        return all_paths

    def get_node_path(self, relate_path_list,
                      start_node_list):
        nxDiG = nx.DiGraph()
        for edge in relate_path_list:
            source, destination = edge.split("-->")
            nxDiG.add_edge(source, destination)
        all_node_paths = []
        for start_node in start_node_list:
            paths = self.find_all_paths_from_start(nxDiG, start_node)
            node_paths = ["-->".join(path) for path in paths]
            all_node_paths += node_paths
        return all_node_paths

    def get_longest_node_path(self, all_node_paths,
                              start_node_list,
                              end_node_list):
        alternative_path_list = []
        for single_node_path in all_node_paths:
            # all_node_path = all_node_paths[0]
            path_start_node = single_node_path.split("-->")[0]
            path_end_node = single_node_path.split("-->")[-1]
            if path_start_node in start_node_list:
                if path_end_node in end_node_list:
                    alternative_path_list.append(single_node_path)
                else:
                    pass
            else:
                pass

        return alternative_path_list

    def nodes_relate(self):
        # get start/end node
        start_node_list, end_node_list = self.search_node_se()
        # get relate path list
        node_name_list = list(self.nodes["node_name"])
        relate_path_list = []
        for node_name in node_name_list:
            relate_path_l = self.get_next_node(node_name)
            relate_path_list += relate_path_l
        # get all node relate paths
        all_node_paths = self.get_node_path(relate_path_list, start_node_list)
        # get the longest node path
        return all_node_paths, end_node_list


if __name__ == '__main__':
    from scripts.graph.graph_manage import GraphManage
    pbtxt_path = "conf/scene/ddos_detection/ddos_anomaly_detection_b1m2.pbtxt"
    GM = GraphManage(pbtxt_path)
    input_stream, output_stream, nodes = GM.graph_config()

    NR = NodesRelate(nodes, input_stream, output_stream)
    all_node_paths, end_node_list = NR.nodes_relate()
    print("nodes_relate_res: \n", all_node_paths)

    # node path test
    # relate_path_list = ['node0-->node3', 'node1-->node4', 'node2-->node5',
    #                     'node3-->node4', 'node4-->node5', 'node5-->node6']
    # start_node_list = ['node0', 'node1', 'node2']
    # test = NR.get_node_path(relate_path_list, start_node_list)
    # print("test: \n", test)
    
