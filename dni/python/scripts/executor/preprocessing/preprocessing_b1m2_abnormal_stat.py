# -*- coding: utf-8 -*-
# @Author: Jelly
# @Email: jellyHello@163.com
# @Time: 2024-03-25 10:10:57

import pandas as pd

class B1M2AbnormalStatPreprocess:
    def __init__(self, abnormal_stat_latest):
        self.abnormal_stat_latest = abnormal_stat_latest

    def packet_stat_process(self, statProcessDF):
        # add packet feature
        packet_addCols = []
        if len(packet_addCols) >= 1:
            packet_cols = ['mark', 'hostNicSign', 'ip']
            packetStatAddDF = statProcessDF[packet_cols].copy()
        else:
            packetStatAddDF = pd.DataFrame()
        return packetStatAddDF

    def netdev_stat_process(self, statProcessDF):
        # add netdev feature
        netdev_addCols = ['inMbps', 'inKpps', 'outMbps', 'outKpps',
                          'inMbpsRatio', 'outMbpsRatio',
                          'dmsDropMbps', 'dmsDropKpps']
        if len(netdev_addCols) >= 1:
            netdev_cols = ['mark', 'hostNicSign', 'ip']
            netdevStatAddDF = statProcessDF[netdev_cols].copy()
            # compute bps/pps -> Mbps/Kpps (inMbps, inKpps, outMbps, outKpps)
            # print("Q1:", abnormal_stat_latest[['mark', 'netif', 'incrbytesRecv', 'incrpacketRecv',
            # 'incrbytesSent', 'incrpacketSent']])
            inMbpsL = [round(int(temp) * 8 / (1000 * 1000), 4) if temp is not None else 0
                       for temp in statProcessDF['incrbytesRecv']]
            netdevStatAddDF['inMbps'] = inMbpsL
            inKppsL = [round(int(temp) / 1000, 4) if temp is not None else 0
                       for temp in statProcessDF['incrpacketRecv']]
            netdevStatAddDF['inKpps'] = inKppsL
            outMbpsL = [round(int(temp) * 8 / (1000 * 1000), 4) if temp is not None else 0
                        for temp in statProcessDF['incrbytesSent']]
            netdevStatAddDF['outMbps'] = outMbpsL
            outKppsL = [round(int(temp) / 1000, 4) if temp is not None else 0
                        for temp in statProcessDF['incrpacketSent']]
            netdevStatAddDF['outKpps'] = outKppsL
            # dms drop stats
            dmsDropCols = [temp for temp in list(statProcessDF.columns) if "dms" in temp]
            if len(dmsDropCols) >= 1:
                dmsDropMbpsL = [round(int(temp) * 8 / (1000 * 1000), 4) if temp is not None else 0
                                for temp in statProcessDF['incrdmsbytesDrp']]
                netdevStatAddDF['dmsDropMbps'] = dmsDropMbpsL
                dmsDropKppsL = [round(int(temp) / 1000, 4) if temp is not None else 0
                                for temp in statProcessDF['incrdmspktsDrp']]
                netdevStatAddDF['dmsDropKpps'] = dmsDropKppsL
            else:
                netdevStatAddDF['dmsDropMbps'] = 0
                netdevStatAddDF['dmsDropKpps'] = 0
            # compute bandwidth ratio, inMbpsRatio, outMbpsRatio
            speedL = list(statProcessDF['speed'])
            speedV_L = []
            for speed in speedL:
                # speed = speedL[0]
                if "Mb" in speed:
                    speedV = int(speed.split("Mb")[0])
                elif "Gb" in speed:
                    speedV = int(speed.split("Gb")[0]) * 1024
                elif "Kb" in speed:
                    speedV = int(int(speed.split("Kb")[0]) / 1024)
                elif "void" in speed:
                    # unknown nic speed is 10 Gb/s by default
                    speedV = 10000
                else:
                    # format abnormal
                    speedV = int("".join([temp for temp in speed if temp.isdigit()]))
                speedV_L.append(speedV)
            # inMbpsRatio
            netdevStatAddDF["inMbpsRatio"] = [round(inMbpsL[idx] / speedV_L[idx], 4)
                                              for idx in range(len(inMbpsL))]
            netdevStatAddDF["outMbpsRatio"] = [round(outMbpsL[idx] / speedV_L[idx], 4)
                                               for idx in range(len(outMbpsL))]
        else:
            netdevStatAddDF = pd.DataFrame()
        return netdevStatAddDF

    def resource_stat_process(self, statProcessDF):
        # add resource feature
        resource_addCols = []
        if len(resource_addCols) >= 1:
            resource_cols = ['mark']
            resourceStatAddDF = statProcessDF[resource_cols].copy()
        else:
            resourceStatAddDF = pd.DataFrame()
        return resourceStatAddDF

    def form_process_df(self):
        # deal with noneType
        fillLabelCols = ['packet_time', 'ip', 'mark', 'netif', 'nodeid',
                         'pcapName', 'speed', 'host', 'hostNicSign',
                         'netdev_time', 'resource_time', 'hostSign', 'mgmtIP']
        statCols = list(self.abnormal_stat_latest.columns)
        fillPacketCols = [temp for temp in list(set(statCols) - set(fillLabelCols)) if temp.find("count") >= 0]
        fillNetdevCols = [temp for temp in list(set(statCols) - set(fillLabelCols))
                          if "Sent" in temp or
                          "Recv" in temp or
                          "Drp" in temp]
        fillResourceCols = [temp for temp in list(set(statCols) - set(fillLabelCols))
                            if "cur_" in temp or
                            "incr_" in temp]
        statProcessDF = self.abnormal_stat_latest.copy()
        statProcessDF[fillLabelCols] = statProcessDF[fillLabelCols].fillna("void")
        statProcessDF[fillPacketCols] = statProcessDF[fillPacketCols].fillna(0)
        statProcessDF[fillNetdevCols] = statProcessDF[fillNetdevCols].fillna(0)
        statProcessDF[fillResourceCols] = statProcessDF[fillResourceCols].fillna(0)

        # add packet feature
        packetStatAddDF = self.packet_stat_process(statProcessDF)
        if len(packetStatAddDF) >= 1:
            statProcessDF = pd.merge(statProcessDF, packetStatAddDF,
                                     on=['mark', 'hostNicSign', 'ip'])
        else:
            pass
        # add netdev feature
        netdevStatAddDF = self.netdev_stat_process(statProcessDF)
        if len(netdevStatAddDF) >= 1:
            statProcessDF = pd.merge(statProcessDF, netdevStatAddDF,
                                     on=['mark', 'hostNicSign', 'ip'])
        else:
            pass
        # add resource feature
        resourceStatAddDF = self.resource_stat_process(statProcessDF)
        if len(resourceStatAddDF) >= 1:
            statProcessDF = pd.merge(statProcessDF, resourceStatAddDF, on=['mark'])
        else:
            pass
        return statProcessDF

    def b1m2_stat_preprocess_acquire(self):
        statProcessDF = self.form_process_df()
        node_output = {
            "b1m2_stat_preprocess": statProcessDF
        }
        return node_output


if __name__ == '__main__':
    # abnormal_stat_latest = pd.DataFrame()
    abnormal_stat_latest = graph_outputs["abnormal_stat_latest"]
    B12ASP = B1M2AbnormalStatPreprocess(abnormal_stat_latest)
    node_output = B12ASP.b1m2_stat_preprocess_acquire()
