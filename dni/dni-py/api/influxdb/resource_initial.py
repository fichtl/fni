# -*- coding: utf-8 -*-
# @Author: Jelly
# @Email: jellyHello@163.com
# @Time: 11/2/23 2:50 PM

import influxdb
import pandas as pd
from conf.system.param_parse import param_parse


class ResourceInitial:
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


if __name__ == '__main__':
    # data display setting
    import numpy as np
    import warnings
    pd.set_option('display.width', None)
    pd.set_option('display.max_colwidth', None)
    warnings.filterwarnings('ignore')
    np.set_printoptions(suppress=False)

    RI = ResourceInitial()
    resource_initial = RI.get_resource_initial()
    print(resource_initial.head())


