# -*- coding: utf-8 -*-
# @Author: Jelly
# @Email: jellyHello@163.com
# @Time: 11/2/23 2:50 PM

import pytz
import datetime
import influxdb
import pandas as pd
from conf.system.param_parse import param_parse


class ResourceStat:
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

    def get_resource_query(self):
        # protected info
        analyHost = ["host='" + temp + "'" for temp in analyHostName]
        selHost = " or ".join(analyHost)
        query_resource = "SELECT * FROM resource WHERE %s GROUP BY host ORDER BY time DESC LIMIT %s" \
                       % (selHost, influxdbSelectNum)
        return query_resource

    def get_resource_initial(self):
        self.get_param()
        self.get_wanted_data()
        # get resource data
        query_resource = self.get_resource_query()
        host = influxdbSndingLogin.split(",")[0]
        port = influxdbSndingLogin.split(",")[1]
        database = influxdbSndingLogin.split(",")[2]

        try:
            client = influxdb.InfluxDBClient(host=host, port=port, database=database)
            resourceInitialData = client.query(query_resource)
            resourceInitialDF1 = pd.DataFrame(list(resourceInitialData.get_points()))
            resourceInitialDF = resourceInitialDF1.copy()
            resourceInitialDF["host"] = [temp.split("#")[0] for temp in resourceInitialDF1["mark"]]
            resourceInitialDF = resourceInitialDF.sort_values(by="time", ascending=False)
            return resourceInitialDF
        except Exception as e:
            resourceInitialDF = pd.DataFrame()
            return resourceInitialDF

    def get_latest_resource(self, resourceInitialDF):
        # select latest(1) resource initial dataframe
        resourceInitialDF["hostSign"] = resourceInitialDF["host"]
        resourceHostTypeList = list(set(resourceInitialDF["hostSign"]))
        resourceStatLatestList = []
        for resourceHostType in resourceHostTypeList:
            # nicSign = resourceNicTypeList[0]
            resourceNicTypeDF = resourceInitialDF[resourceInitialDF["hostSign"] == resourceHostType]
            resourceNicTypeS = list(resourceNicTypeDF.iloc[0])
            resourceStatLatestList.append(resourceNicTypeS)
        resourceStatLatestCol = resourceInitialDF.columns
        resourceStatLatestDF = pd.DataFrame(resourceStatLatestList, columns=resourceStatLatestCol)
        return resourceStatLatestDF, resourceHostTypeList

    def match_wanted_resource(self, resourceInitialDF):
        # get influxdb resource
        resourceStatLatestDF, resourceHostTypeList = self.get_latest_resource(resourceInitialDF)
        # get protected host nic info
        # analyNicInfoDF, protHostNicList = self.get_wanted_host()
        # 1/3) match analysis resource data condition
        if len(selNicInfoDF) > 0:
            selHostList = list(set([temp.split("#")[0] for temp in selNicNameList]))
            resourceMatchSameRes = list(set(selHostList) & set(resourceHostTypeList))
            resourceMatchDiffNum = len(selHostList) - len(resourceHostTypeList)
            matchNum = len(selHostList)
        else:
            # assign
            resourceMatchSameRes = resourceHostTypeList
            resourceMatchDiffNum = 0
            matchNum = "pass"
        # 2/3) get match data
        if resourceMatchDiffNum >= 1:
            # collect nic less
            # resourceMatchDiffList = list(set(selHostList) - set(resourceHostTypeList))
            resourceStatMatchLatestDF1 = resourceStatLatestDF[resourceStatLatestDF
            ["hostSign"].isin(resourceMatchSameRes)]
        elif resourceMatchDiffNum == 0:
            # collect nic equal
            # resourceMatchDiffList = []
            resourceStatMatchLatestDF1 = resourceStatLatestDF.copy()
        else:
            # collect nic more
            # resourceMatchDiffList = list(set(resourceHostTypeList) - set(selHostList))
            resourceStatMatchLatestDF1 = resourceStatLatestDF[resourceStatLatestDF
            ["hostSign"].isin(resourceMatchSameRes)]

        # 3/3) sel anlaysis host(mgmtIP)
        resourceStatMatchLatestDF1["mgmtIP"] = [temp.split("#")[1] for temp in resourceStatMatchLatestDF1["mark"]]
        if selAnalyMgmtIP == "no":
            resourceStatMatchLatestDF3 = resourceStatMatchLatestDF1.copy()
        else:
            resourceStatMatchLatestDF2 = resourceStatMatchLatestDF1.copy()
            selMgmtIPRuleStr = selAnalyMgmtIP.replace(",", "|")
            resourceStatMatchLatestDF3 = resourceStatMatchLatestDF2[
                resourceStatMatchLatestDF2["mgmtIP"].str.contains(selMgmtIPRuleStr)]

        # 4/4)
        resourceStatMatchLatestDF = resourceStatMatchLatestDF3.copy()
        resourceStatMatchLatestDF["cur_cpu"] = [round(temp, 4) for temp in resourceStatMatchLatestDF3["cur_cpu"]]

        resourceMatchHost = list(resourceStatMatchLatestDF["hostSign"])
        # rename time to resource_time
        resourceStatMatchLatestDF = resourceStatMatchLatestDF.rename(columns={"time": "resource_time"})
        return resourceStatMatchLatestDF, resourceMatchHost

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

    def get_resource_data(self):
        resourceInitialDF = self.get_resource_initial()
        resourceStatMatchLatestDF, resourceMatchHost = self.match_wanted_resource(resourceInitialDF)
        print(" resourceStatMatchLatestDF:", resourceStatMatchLatestDF.shape)
        # deal time 0 timezone to beijing timezone
        resourceStatMatchLatestDF["resource_time"] = self.time_zone_adjust(resourceStatMatchLatestDF["resource_time"])
        return resourceStatMatchLatestDF


if __name__ == '__main__':
    # data display setting
    import numpy as np
    import warnings
    pd.set_option('display.width', None)
    pd.set_option('display.max_colwidth', None)
    warnings.filterwarnings('ignore')
    np.set_printoptions(suppress=False)

    RS = ResourceStat()
    resourceStatMatchLatestDF = RS.get_resource_data()
    print(resourceStatMatchLatestDF.head())


