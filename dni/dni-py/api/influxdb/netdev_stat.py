# -*- coding: utf-8 -*-
# @Author: Jelly
# @Email: jellyHello@163.com
# @Time: 11/2/23 2:50 PM

import pytz
import datetime
import influxdb
import pandas as pd
from conf.system.param_parse import param_parse


class NetdevStat:
    def get_param(self):
        global influxdbSndingLogin
        global hostLoginPath
        global hostInfoBool
        global hostInfoMatch
        global hostInfoPath
        global selAnalyMgmtIP
        global selAnalyDataIP
        global checkDataUpdate
        global dataTimePath
        global influxdbSelectNum
        config = param_parse()
        influxdbSelectNum = config.get('database', 'influxdb_select_num')
        influxdbSndingLogin = config.get('database', 'influxdb_snding_login')
        hostLoginPath = config.get('path', 'host_login_path')
        hostInfoBool = config.get('base', 'host_info_bool')
        hostInfoMatch = config.get('base', 'host_info_match')
        hostInfoPath = config.get('path', 'host_info_path')
        selAnalyMgmtIP = config.get('base', 'sel_analy_mgmtip')
        selAnalyDataIP = config.get('base', 'sel_analy_dataip')
        checkDataUpdate = config.get('base', 'check_data_update')
        dataTimePath = config.get('path', 'time_save_path')

    def get_host_login(self):
        # get all Host Login DataFrame
        if "txt" in hostLoginPath:
            allHostLoginDF = pd.read_csv(hostLoginPath, header=0, sep=' ')
        else:
            allHostLoginDF = pd.read_csv(hostLoginPath, header=0, sep=',')
        # select protectee host
        detectRoleList = ['protected', 'A10', 'defender']
        analyLoginDF = allHostLoginDF[allHostLoginDF['role'].isin(detectRoleList)]
        return allHostLoginDF, analyLoginDF

    def get_wanted_data(self):
        global selNicInfoDF
        global selNicNameList
        global analyHostName
        _, analyLoginDF = self.get_host_login()
        analyHostName = list(analyLoginDF["hostName"])
        # judge whether the host information can be obtained
        if hostInfoBool == "yes":
            # select the matching mode between the real-time statistic data
            # and the known host info
            if hostInfoMatch == "check":
                # get all Host Info DataFrame
                allHostInfoDF = pd.read_csv(hostInfoPath, header=0, sep=',')
                # merge protected host info Dataframe
                # ["hostName", "userName", "mgmtIP"]
                merge_cols = list(set(analyLoginDF.columns) & set(allHostInfoDF))
                analyNicInfoDF = pd.merge(analyLoginDF, allHostInfoDF, on=merge_cols)
                analyNicInfoDF["nicIP"] = [temp.split("/")[0] for temp in analyNicInfoDF["nicIpMask"]]
                analyNicInfoDF["hostNicSign"] = analyNicInfoDF[["hostName", "nicName"]] \
                    .apply(lambda x: "#".join(x), axis=1)
                # select target analysis ip
                if selAnalyMgmtIP == "no":
                    selNicInfoDF = analyNicInfoDF.copy()
                    selNicNameList = list(set(selNicInfoDF["hostNicSign"]))
                else:
                    selMgmtIPRuleStr = selAnalyMgmtIP.replace(",", "|")
                    selHostInfoDF = analyNicInfoDF[analyNicInfoDF["mgmtIP"].str.contains(selMgmtIPRuleStr)]
                    if selAnalyDataIP == "no":
                        selNicInfoDF = selHostInfoDF.copy()
                    else:
                        selDataIPRuleStr = selAnalyDataIP.replace(",", "|")
                        selNicInfoDF = selHostInfoDF[selHostInfoDF["nicIP"].str.contains(selDataIPRuleStr)]
                    selNicNameList = list(set(selNicInfoDF["hostNicSign"]))
            else:
                # hostInfoMatch == "pass"
                selNicInfoDF = pd.DataFrame()
                selNicNameList = []
        else:
            selNicInfoDF = pd.DataFrame()
            selNicNameList = []

    def get_netdev_query(self):
        # protected info
        analyHost = ["host='" + temp + "'" for temp in analyHostName]
        selHost = " or ".join(analyHost)
        query_netdev = "SELECT * FROM netdev WHERE %s GROUP BY host ORDER BY time DESC LIMIT %s" \
                       % (selHost, influxdbSelectNum)
        return query_netdev

    def get_netdev_initial(self):
        self.get_param()
        self.get_wanted_data()
        # get netdev data
        query_netdev = self.get_netdev_query()
        host = influxdbSndingLogin.split(",")[0]
        port = influxdbSndingLogin.split(",")[1]
        database = influxdbSndingLogin.split(",")[2]

        try:
            client = influxdb.InfluxDBClient(host=host, port=port, database=database)
            netdevInitialData = client.query(query_netdev)
            netdevInitialDF1 = pd.DataFrame(list(netdevInitialData.get_points()))
            netdevInitialDF = netdevInitialDF1.copy()
            netdevInitialDF["host"] = [temp.split("#")[0] for temp in netdevInitialDF1["mark"]]
            netdevInitialDF = netdevInitialDF.sort_values(by="time", ascending=False)
            return netdevInitialDF
        except Exception as e:
            netdevInitialDF = pd.DataFrame()
            return netdevInitialDF

    def get_latest_netdev(self, netdevInitialDF):
        # netdevInitialDF, netdevInitialDF, resourceInitialDF = self.get_influxdb_data()
        # select latest(1) netdev initial dataframe
        netdevInitialDF["hostNicSign"] = netdevInitialDF[["host", "netif"]] \
            .apply(lambda x: "#".join(x), axis=1)
        netdevNicTypeList = list(set(netdevInitialDF["hostNicSign"]))
        netdevStatLatestList = []
        for netdevNicType in netdevNicTypeList:
            # netdevNicType = netdevNicTypeList[0]
            netdevNicTypeDF = netdevInitialDF[netdevInitialDF["hostNicSign"] == netdevNicType]
            netdevNicTypeS = list(netdevNicTypeDF.iloc[0])
            netdevStatLatestList.append(netdevNicTypeS)
        netdevStatLatestCol = netdevInitialDF.columns
        netdevStatLatestDF = pd.DataFrame(netdevStatLatestList, columns=netdevStatLatestCol)
        return netdevStatLatestDF, netdevNicTypeList

    def match_wanted_netdev(self, netdevInitialDF):
        # get influxdb netdev
        netdevStatLatestDF, netdevNicTypeList = self.get_latest_netdev(netdevInitialDF)
        # 1/6) match analysis netdev data condition
        if len(selNicInfoDF) > 0:
            # exist wanted nic match info
            netdevMatchSameRes = list(set(selNicNameList) & set(netdevNicTypeList))
            netdevMatchDiffNum = len(selNicNameList) - len(netdevNicTypeList)
            matchNum = len(selNicNameList)
        else:
            # assign
            netdevMatchSameRes = netdevNicTypeList
            netdevMatchDiffNum = 0
            matchNum = "pass"
        # 2/6) get match data
        if netdevMatchDiffNum >= 1:
            # collect nic less
            # netdevMatchDiffList = list(set(selNicNameList) - set(netdevNicTypeList))
            netdevStatMatchLatestDF1 = netdevStatLatestDF[
                netdevStatLatestDF["hostNicSign"].isin(netdevMatchSameRes)]
        elif netdevMatchDiffNum == 0:
            # collect nic equal
            # netdevMatchDiffList = []
            netdevStatMatchLatestDF1 = netdevStatLatestDF.copy()
        else:
            # collect nic more
            # netdevMatchDiffList = list(set(netdevNicTypeList) - set(selNicNameList))
            netdevStatMatchLatestDF1 = netdevStatLatestDF[
                netdevStatLatestDF["hostNicSign"].isin(netdevMatchSameRes)]

        # 3/6) rename time to netdev_time
        netdevStatMatchLatestDF1 = netdevStatMatchLatestDF1.rename(
            columns={"time": "netdev_time"})

        # 4/6) deal with ip(=None)
        netdevStatMatchLatestDF2 = netdevStatMatchLatestDF1.copy()
        netdevStatMatchLatestDF2["ip"] = [temp if temp and "." in temp else "void"
                                          for temp in netdevStatMatchLatestDF2["ip"]]
        # 5/6) deal with speed(=None)
        netdevStatMatchLatestDF3 = netdevStatMatchLatestDF2.copy()
        netdevStatMatchLatestDF3["speed"] = [temp if temp and "b/s" in temp else "void"
                                             for temp in netdevStatMatchLatestDF3["speed"]]

        # 6/6) select analysis ip
        netdevStatMatchLatestDF3["mgmtIP"] = [temp.split("#")[1] for temp in netdevStatMatchLatestDF3["mark"]]
        if selAnalyMgmtIP == "no":
            netdevStatMatchLatestDF = netdevStatMatchLatestDF3.copy()
        else:
            selMgmtIPRuleStr = selAnalyMgmtIP.replace(",", "|")
            netdevStatMatchLatestDF4 = netdevStatMatchLatestDF3.copy()
            netdevStatMatchLatestDF5 = netdevStatMatchLatestDF4[
                netdevStatMatchLatestDF4["mgmtIP"].str.contains(selMgmtIPRuleStr)]
            if selAnalyDataIP == "no":
                netdevStatMatchLatestDF = netdevStatMatchLatestDF5.copy()
            else:
                selDataIPRuleStr = selAnalyDataIP.replace(",", "|")
                netdevStatMatchLatestDF = netdevStatMatchLatestDF5[
                    netdevStatMatchLatestDF5["ip"].str.contains(selDataIPRuleStr)]
        netdevMatchHostNic = list(netdevStatMatchLatestDF["hostNicSign"])
        return netdevStatMatchLatestDF, netdevMatchHostNic

    def time_zone_adjust(self, timestamp):
        # timestamp = analyNicLatestDF["time"]
        timestamp_bj = []
        for utc_time_str in timestamp:
            # utc_time_str = timestamp[0]
            if "." in utc_time_str:
                # time format: "%Y-%m-%dT%H:%M:%S.%fZ"
                utc_time = datetime.datetime.strptime(utc_time_str, "%Y-%m-%dT%H:%M:%S.%fZ")
                # create 0 timezone object
                utc_timezone = pytz.timezone('UTC')
                utc_datetime = utc_timezone.localize(utc_time)
                # switch beijing time zone
                bj_timezone = pytz.timezone('Asia/Shanghai')
                bj_datetime = utc_datetime.astimezone(bj_timezone)
                bj_time_str = bj_datetime.strftime("%Y-%m-%d %H:%M:%S.%f")
                timestamp_bj.append(bj_time_str)
            else:
                # time format: "%Y-%m-%dT%H:%M:%SZ"
                utc_time = datetime.datetime.strptime(utc_time_str, "%Y-%m-%dT%H:%M:%SZ")
                # create 0 timezone object
                utc_timezone = pytz.timezone('UTC')
                utc_datetime = utc_timezone.localize(utc_time)
                # switch beijing time zone
                bj_timezone = pytz.timezone('Asia/Shanghai')
                bj_datetime = utc_datetime.astimezone(bj_timezone)
                bj_time_str = bj_datetime.strftime("%Y-%m-%d %H:%M:%S.%f")
                timestamp_bj.append(bj_time_str)
        # print(timestamp_bj)
        return timestamp_bj

    def get_netdev_data(self):
        netdevInitialDF = self.get_netdev_initial()
        netdevStatMatchLatestDF, netdevMatchHostNic = self.match_wanted_netdev(netdevInitialDF)
        print(" netdevStatMatchLatestDF:", netdevStatMatchLatestDF.shape)
        # deal time 0 timezone to beijing timezone
        netdevStatMatchLatestDF["netdev_time"] = self.time_zone_adjust(netdevStatMatchLatestDF["netdev_time"])
        return netdevStatMatchLatestDF


if __name__ == '__main__':
    # data display setting
    import numpy as np
    import warnings
    pd.set_option('display.width', None)
    pd.set_option('display.max_colwidth', None)
    warnings.filterwarnings('ignore')
    np.set_printoptions(suppress=False)
    
    NS = NetdevStat()
    netdevStatMatchLatestDF = NS.get_netdev_data()
    print(netdevStatMatchLatestDF.head())
