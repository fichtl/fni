# -*- coding: utf-8 -*-
# @Author: Jelly
# @Email: jellyHello@163.com
# @Time: 11/2/23 2:50 PM

import os.path
import pytz
import datetime
import influxdb
import pandas as pd
from conf.system.param_parse import param_parse


class PacketStat:
    def get_param(self):
        global influxdbSelectNum
        global influxdbSndingLogin
        global hostLoginPath
        global hostInfoBool
        global hostInfoMatch
        global hostInfoPath
        global selAnalyMgmtIP
        global selAnalyDataIP
        global checkDataUpdate
        global dataTimePath
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

    def get_packet_query(self):
        # protected info
        analyHost = ["host='" + temp + "'" for temp in analyHostName]
        selHost = " or ".join(analyHost)
        query_packet = "SELECT * FROM packet WHERE %s GROUP BY host ORDER BY time DESC LIMIT %s" \
                       % (selHost, influxdbSelectNum)
        return query_packet

    def get_packet_initial(self):
        self.get_param()
        self.get_wanted_data()
        # get packet data
        query_packet = self.get_packet_query()
        host = influxdbSndingLogin.split(",")[0]
        port = influxdbSndingLogin.split(",")[1]
        database = influxdbSndingLogin.split(",")[2]

        try:
            client = influxdb.InfluxDBClient(host=host, port=port, database=database)
            packetInitialData = client.query(query_packet)
            packetInitialDF1 = pd.DataFrame(list(packetInitialData.get_points()))
            packetInitialDF = packetInitialDF1.copy()
            packetInitialDF["host"] = [temp.split("#")[0] for temp in packetInitialDF1["mark"]]
            packetInitialDF = packetInitialDF.sort_values(by="time", ascending=False)
            return packetInitialDF
        except Exception as e:
            packetInitialDF = pd.DataFrame()
            return packetInitialDF

    def get_latest_packet(self, packetInitialDF):
        # packetInitialDF, netdevInitialDF, resourceInitialDF = self.get_influxdb_data()
        # select latest(1) packet initial dataframe
        packetInitialDF["hostNicSign"] = packetInitialDF[["host", "netif"]] \
            .apply(lambda x: "#".join(x), axis=1)
        packetNicTypeList = list(set(packetInitialDF["hostNicSign"]))
        packetStatLatestList = []
        # check whether the data is updated
        if checkDataUpdate == "yes":
            print(" checking whether the data is the latest...")
            for packetNicType in packetNicTypeList:
                # packetNicType = packetNicTypeList[0]
                packetNicTypeDF = packetInitialDF[packetInitialDF["hostNicSign"] == packetNicType]
                packetNicTypeS = packetNicTypeDF.iloc[0]
                hostNicTime = "_".join([packetNicType, packetNicTypeS["time"]])
                # read hostNicTime save records
                # check
                if os.path.exists(dataTimePath):
                    dataTimeSize = os.path.getsize(dataTimePath)
                    dataTimeSizeM = round(dataTimeSize / (1024*1024), 4)
                    if dataTimeSizeM >= 10:
                        #  clear data when >= 10M
                        with open(dataTimePath, 'w') as f:
                            pass
                    else:
                        pass
                else:
                    with open(dataTimePath, 'w') as f:
                        pass
                # read
                with open(dataTimePath, 'r') as f1:
                    dataTimeRecordTxt = f1.read()
                dataTimeRecord = dataTimeRecordTxt.split(",")
                if hostNicTime in dataTimeRecord:
                    pass
                else:
                    packetNicTypeS1 = list(packetNicTypeDF.iloc[0])
                    packetStatLatestList.append(packetNicTypeS1)
                    # save hostNicTime to dataTimeRecord
                    dataTimeNew = hostNicTime + ","
                    with open(dataTimePath, 'a') as f2:
                        f2.write(dataTimeNew)
        else:
            # print(" the first data is the latest data.")
            for packetNicType in packetNicTypeList:
                # packetNicType = packetNicTypeList[0]
                packetNicTypeDF = packetInitialDF[packetInitialDF["hostNicSign"] == packetNicType]
                packetNicTypeS = list(packetNicTypeDF.iloc[0])
                packetStatLatestList.append(packetNicTypeS)

        packetStatLatestCol = packetInitialDF.columns
        packetStatLatestDF = pd.DataFrame(packetStatLatestList, columns=packetStatLatestCol)
        return packetStatLatestDF, packetNicTypeList

    def match_wanted_packet(self, packetInitialDF):
        # get influxdb packet
        packetStatLatestDF, packetNicTypeList = self.get_latest_packet(packetInitialDF)
        # 1/6) match analysis packet data condition
        if len(selNicInfoDF) > 0:
            # exist wanted nic match info
            packetMatchSameRes = list(set(selNicNameList) & set(packetNicTypeList))
            packetMatchDiffNum = len(selNicNameList) - len(packetNicTypeList)
            matchNum = len(selNicNameList)
        else:
            # assign
            packetMatchSameRes = packetNicTypeList
            packetMatchDiffNum = 0
            matchNum = "pass"
        # 2/6) get match data
        if packetMatchDiffNum >= 1:
            # collect nic less
            # packetMatchDiffList = list(set(selNicNameList) - set(packetNicTypeList))
            packetStatMatchLatestDF1 = packetStatLatestDF[
                packetStatLatestDF["hostNicSign"].isin(packetMatchSameRes)]
        elif packetMatchDiffNum == 0:
            # collect nic equal
            # packetMatchDiffList = []
            packetStatMatchLatestDF1 = packetStatLatestDF.copy()
        else:
            # collect nic more
            # packetMatchDiffList = list(set(packetNicTypeList) - set(selNicNameList))
            packetStatMatchLatestDF1 = packetStatLatestDF[
                packetStatLatestDF["hostNicSign"].isin(packetMatchSameRes)]

        # 3/6) rename time to packet_time
        packetStatMatchLatestDF1 = packetStatMatchLatestDF1.rename(
            columns={"time": "packet_time"})

        # 4/6) deal with ip(=None)
        packetStatMatchLatestDF2 = packetStatMatchLatestDF1.copy()
        packetStatMatchLatestDF2["ip"] = [temp if temp and "." in temp else "void"
                                          for temp in packetStatMatchLatestDF2["ip"]]
        # 5/6) deal with speed(=None)
        packetStatMatchLatestDF3 = packetStatMatchLatestDF2.copy()
        packetStatMatchLatestDF3["speed"] = [temp if temp and "b/s" in temp else "void"
                                             for temp in packetStatMatchLatestDF3["speed"]]

        # 6/6) select analysis ip
        packetStatMatchLatestDF3["mgmtIP"] = [temp.split("#")[1] for temp in packetStatMatchLatestDF3["mark"]]
        if selAnalyMgmtIP == "no":
            packetStatMatchLatestDF = packetStatMatchLatestDF3.copy()
        else:
            selMgmtIPRuleStr = selAnalyMgmtIP.replace(",", "|")
            packetStatMatchLatestDF4 = packetStatMatchLatestDF3.copy()
            packetStatMatchLatestDF5 = packetStatMatchLatestDF4[
                packetStatMatchLatestDF4["mgmtIP"].str.contains(selMgmtIPRuleStr)]
            if selAnalyDataIP == "no":
                packetStatMatchLatestDF = packetStatMatchLatestDF5.copy()
            else:
                selDataIPRuleStr = selAnalyDataIP.replace(",", "|")
                packetStatMatchLatestDF = packetStatMatchLatestDF5[
                    packetStatMatchLatestDF5["ip"].str.contains(selDataIPRuleStr)]
        packetMatchHostNic = list(packetStatMatchLatestDF["hostNicSign"])
        return packetStatMatchLatestDF, packetMatchHostNic

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

    def get_packet_data(self):
        packetInitialDF = self.get_packet_initial()
        packetStatMatchLatestDF, packetMatchHostNic = self.match_wanted_packet(packetInitialDF)
        print(" packetStatMatchLatestDF:", packetStatMatchLatestDF.shape)
        # deal time 0 timezone to beijing timezone
        packetStatMatchLatestDF["packet_time"] = self.time_zone_adjust(packetStatMatchLatestDF["packet_time"])
        return packetStatMatchLatestDF


if __name__ == '__main__':
    # data display setting
    import numpy as np
    import warnings
    pd.set_option('display.width', None)
    pd.set_option('display.max_colwidth', None)
    warnings.filterwarnings('ignore')
    np.set_printoptions(suppress=False)

    PS = PacketStat()
    packetStatMatchLatestDF = PS.get_packet_data()
    print(packetStatMatchLatestDF.head())

