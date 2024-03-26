# -*- coding: utf-8 -*-
# @Author: Jelly
# @Email: jellyHello@163.com
# @Time: 11/2/23 2:50 PM

import influxdb
import pandas as pd
from conf.system.param_parse import param_parse


class PacketInitial:
    def get_param(self):
        global influxdbSelectNum
        global influxdbSndingLogin
        global hostLoginPath
        config = param_parse()
        influxdbSelectNum = config.get('database', 'influxdb_select_num')
        influxdbSndingLogin = config.get('database', 'influxdb_snding_login')
        hostLoginPath = config.get('path', 'host_login_path')

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
        global analyHostName
        _, analyLoginDF = self.get_host_login()
        analyHostName = list(analyLoginDF["hostName"])

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


if __name__ == '__main__':

    PI = PacketInitial()
    packetInitialDF = PI.get_packet_initial()
    print(packetInitialDF.head())

