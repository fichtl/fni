# -*- coding: utf-8 -*-
# @Author: Jelly
# @Email: jellyHello@163.com
# @Time: 2024-03-23 16:29:18

from dask import delayed

class Execute:
    def __init__(self, executor_name, node_inputs):
        self.executor_name = executor_name
        self.node_inputs = node_inputs

    def execute_output(self):
        if self.executor_name == "telemetry_packet_stat_latest":
            from scripts.executor.telemetry.telemetry_packet_stat_latest \
                import PacketStatLatest
            # packet_initial = node_inputs[0]
            PSL = PacketStatLatest(*self.node_inputs)
            # packet_latest = PSL.get_packet_stat()
            delayed_res = delayed(PSL.get_packet_stat()).compute()
        elif self.executor_name == "decision_a1m2_result":
            from scripts.executor.decision.decision_a1m2_result \
                import PacketStatStatus
            PSS = PacketStatStatus(*self.node_inputs)
            # normal_status, abnormal_basis = PSS.assess_packet_status()
            delayed_res = delayed(PSS.assess_packet_status()).compute()
        elif self.executor_name == "telemetry_abnormal_nic":
            from scripts.executor.telemetry.telemetry_abnormal_nic import AbnormalNic
            AN = AbnormalNic(*self.node_inputs)
            # abnormal_nic = AN.determine_abnormal_nic()
            delayed_res = delayed(AN.determine_abnormal_nic()).compute()
        elif self.executor_name == "telemetry_abnormal_packet_latest":
            from scripts.executor.telemetry.telemetry_abnormal_packet_latest import AbnormalPacketLatest
            APL = AbnormalPacketLatest(*self.node_inputs)
            delayed_res = delayed(APL.abnormal_packet_acquire()).compute()
        elif self.executor_name == "telemetry_abnormal_netdev_latest":
            from scripts.executor.telemetry.telemetry_abnormal_netdev_latest import AbnormalNetdevLatest
            ANL = AbnormalNetdevLatest(*self.node_inputs)
            delayed_res = delayed(ANL.abnormal_netdev_acquire()).compute()
        elif self.executor_name == "telemetry_abnormal_resource_latest":
            from scripts.executor.telemetry.telemetry_abnormal_resource_latest import AbnormalResourceLatest
            ARL = AbnormalResourceLatest(*self.node_inputs)
            delayed_res = delayed(ARL.abnormal_resource_acquire()).compute()
        elif self.executor_name == "telemetry_abnormal_stat_merge":
            from scripts.executor.telemetry.telemetry_abnormal_stat_merge import AbnormalStatMerge
            ASM = AbnormalStatMerge(*self.node_inputs)
            delayed_res = delayed(ASM.abnormal_stat_merge()).compute()
        elif self.executor_name == "preprocessing_b1m2_abnormal_stat":
            from scripts.executor.preprocessing.preprocessing_b1m2_abnormal_stat import B1M2AbnormalStatPreprocess
            B12ASP = B1M2AbnormalStatPreprocess(*self.node_inputs)
            delayed_res = delayed(B12ASP.b1m2_stat_preprocess_acquire()).compute()
        else:
            print("# please enter a correct node executor name (%s)!" % self.executor_name)
            delayed_res = {}
        return delayed_res


if __name__ == "__main__":
    executor_name = list(node["executor_name"])[0]
    E = Execute(executor_name, node_inputs)
    delayed_res = E.execute_output()
