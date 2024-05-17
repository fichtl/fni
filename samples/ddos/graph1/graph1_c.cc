#include <chrono>
#include <thread>

#include "dni/framework/framework.h"
#include "spdlog/spdlog.h"

void inject_after(dni::Graph* g, int after, int n, int interval)
{
        std::this_thread::sleep_for(std::chrono::milliseconds(after));

        // packet: countTotal
        double_t countTotal = 13000000;
        std::vector<double_t> countTotal_th = {4000, 10000, 100000, 15000000};
        std::vector<double_t> countTotal_score = {0, 0.4, 0.6, 0.8, 1.2};

        // packet: countType_length
        double_t countType_length = 2;
        std::vector<double_t> cond_countType_length = {2001};
        std::vector<double_t> countType_length_th = {6, 500};
        std::vector<double_t> countType_length_score = {0.2, 0.1};
        std::vector<double_t> cond_countType_length_th = {2000};

        // packet: countType_srcIP
        double_t countType_srcIP = 3;
        std::vector<double_t> cond_countType_srcIP = {2001};
        std::vector<double_t> countType_srcIP_th = {6, 255};
        std::vector<double_t> countType_srcIP_score = {0.4, 0.4};
        std::vector<double_t> cond_countType_srcIP_th = {2000};

        // packet: countType_dstIP
        double_t countType_dstIP = 1;
        std::vector<double_t> cond_countType_dstIP = {2001};
        std::vector<double_t> countType_dstIP_th = {6, 255};
        std::vector<double_t> countType_dstIP_score = {0.05, 0.2};
        std::vector<double_t> cond_countType_dstIP_th = {2000};

        // packet: countType_protocol
        double_t countType_protocol = 1;
        std::vector<double_t> cond_countType_protocol = {2001};
        std::vector<double_t> countType_protocol_th = {6, 100};
        std::vector<double_t> countType_protocol_score = {0.1, 0.2};
        std::vector<double_t> cond_countType_protocol_th = {2000};

        // packet: countType_srcPortTcp
        double_t countType_srcPortTcp = 1;
        std::vector<double_t> cond_countType_srcPortTcp = {2001};
        std::vector<double_t> countType_srcPortTcp_th = {6, 100};
        std::vector<double_t> countType_srcPortTcp_score = {0.05, 0.2};
        std::vector<double_t> cond_countType_srcPortTcp_th = {2000};

        // packet: countType_dstPortTcp
        double_t countType_dstPortTcp = 1;
        std::vector<double_t> cond_countType_dstPortTcp = {2001};
        std::vector<double_t> countType_dstPortTcp_th = {6, 100};
        std::vector<double_t> countType_dstPortTcp_score = {0.05, 0.2};
        std::vector<double_t> cond_countType_dstPortTcp_th = {2000};

        // packet: countType_srcPortUdp
        double_t countType_srcPortUdp = 1;
        std::vector<double_t> cond_countType_srcPortUdp = {2001};
        std::vector<double_t> countType_srcPortUdp_th = {6, 100};
        std::vector<double_t> countType_srcPortUdp_score = {0.05, 0.2};
        std::vector<double_t> cond_countType_srcPortUdp_th = {2000};

        // packet: countType_dstPortUdp
        double_t countType_dstPortUdp = 1;
        std::vector<double_t> cond_countType_dstPortUdp = {2001};
        std::vector<double_t> countType_dstPortUdp_th = {6, 100};
        std::vector<double_t> countType_dstPortUdp_score = {0.05, 0.2};
        std::vector<double_t> cond_countType_dstPortUdp_th = {2000};


        // netdev: inMbps
        double_t inMbps = 1024.22;
        std::vector<double_t> inMbps_th = {100, 500, 2000, 15000};
        std::vector<double_t> inMbps_score = {0, 0.2, 0.4, 0.6, 1};

        // netdev: inKpps
        double_t inKpps = 10000;
        std::vector<double_t> inKpps_th = {4000, 10000, 100000, 15000000};
        std::vector<double_t> inKpps_score = {0, 0.2, 0.4, 0.6, 1};

        // netdev: outMbps
        double_t outMbps = 10;
        std::vector<double_t> outMbps_th = {100, 500, 2000, 15000};
        std::vector<double_t> outMbps_score = {0, 0.2, 0.4, 0.6, 1};

        // netdev: outKpps
        double_t outKpps = 1;
        std::vector<double_t> outKpps_th = {4000, 10000, 100000, 15000000};
        std::vector<double_t> outKpps_score = {0, 0.2, 0.4, 0.6, 1};

 
        // resource: cur_cpu
        double_t cur_cpu = 30;
        std::vector<double_t> cur_cpu_th = {20, 40, 60, 80};
        std::vector<double_t> cur_cpu_score = {0, 0.4, 0.7, 1, 1.3};

        // resource: incr_mem_swap
        double_t incr_mem_swap = 1;
        std::vector<double_t> incr_mem_swap_th = {30, 100};
        std::vector<double_t> incr_mem_swap_score = {0, 0.2, 0.4};

        // resource: incr_mem_virtual
        double_t incr_mem_virtual = 1;
        std::vector<double_t> incr_mem_virtual_th = {30, 100};
        std::vector<double_t> incr_mem_virtual_score = {0, 0.2, 0.4};

        // resource: incr_mem_full
        double_t incr_mem_full = 1;
        std::vector<double_t> incr_mem_full_th = {30, 100};
        std::vector<double_t> incr_mem_full_score = {0, 0.3, 0.5};

        // resource: incr_tcpconn_semi
        double_t incr_tcpconn_semi = 1;
        std::vector<double_t> incr_tcpconn_semi_th = {30, 100};
        std::vector<double_t> incr_tcpconn_semi_score = {0, 0.4, 0.7};

        // resource: incr_tcpconn_total
        double_t incr_tcpconn_total = 1;
        std::vector<double_t> incr_tcpconn_total_th = {30, 100};
        std::vector<double_t> incr_tcpconn_total_score = {0, 0.4, 0.7};

        // packet: packet_abnormal_judge, 1
        // double_t packet_scores_sum = 0.33;
        std::vector<double_t> packet_scores_sum_th = {1};
        std::vector<double_t> packet_scores_sum_score = {0, 1};

        // netdev: netdev_abnormal_judge, 0.2
        // double_t netdev_scores_sum = 0.1;
        std::vector<double_t> netdev_scores_sum_th = {0.2};
        std::vector<double_t> netdev_scores_sum_score = {0, 1};

        // resource: resource_abnormal_judge, 0.4
        // double_t resource_scores_sum = 0.15;
        std::vector<double_t> resource_scores_sum_th = {0.4};
        std::vector<double_t> resource_scores_sum_score = {0, 1};



        for (int i = 0; i < n; i++)
        {
                SPDLOG_DEBUG(
                    "send Datum({}) to graph g({:p})", dni::Datum(stat), fmt::ptr(g));

                // packet: countTotal
                g->AddDatumToInputStream("countTotal", dni::Datum(countTotal));
                g->AddDatumToInputSideData("countTotal_th", dni::Datum(countTotal_th));
                g->AddDatumToInputSideData("countTotal_score", dni::Datum(countTotal_score));

                // packet: countType_length
                g->AddDatumToInputStream("countType_length", dni::Datum(countType_length));
                g->AddDatumToInputStream("cond_countType_length", dni::Datum(cond_countType_length));
                g->AddDatumToInputSideData("countType_length_th", dni::Datum(countType_length_th));
                g->AddDatumToInputSideData("countType_length_score", dni::Datum(countType_length_score));
                g->AddDatumToInputSideData("cond_countType_length_th", dni::Datum(cond_countType_length_th));

                // packet: countType_srcIP
                g->AddDatumToInputStream("countType_srcIP", dni::Datum(countType_srcIP));
                g->AddDatumToInputStream("cond_countType_srcIP", dni::Datum(cond_countType_srcIP));
                g->AddDatumToInputSideData("countType_srcIP_th", dni::Datum(countType_srcIP_th));
                g->AddDatumToInputSideData("countType_srcIP_score", dni::Datum(countType_srcIP_score));
                g->AddDatumToInputSideData("cond_countType_srcIP_th", dni::Datum(cond_countType_srcIP_th));

                // packet: countType_dstIP
                g->AddDatumToInputStream("countType_dstIP", dni::Datum(countType_dstIP));
                g->AddDatumToInputStream("cond_countType_dstIP", dni::Datum(cond_countType_dstIP));
                g->AddDatumToInputSideData("countType_dstIP_th", dni::Datum(countType_dstIP_th));
                g->AddDatumToInputSideData("countType_dstIP_score", dni::Datum(countType_dstIP_score));
                g->AddDatumToInputSideData("cond_countType_dstIP_th", dni::Datum(cond_countType_dstIP_th));

                // packet: countType_protocol
                g->AddDatumToInputStream("countType_protocol", dni::Datum(countType_protocol));
                g->AddDatumToInputStream("cond_countType_protocol", dni::Datum(cond_countType_protocol));
                g->AddDatumToInputSideData("countType_protocol_th", dni::Datum(countType_protocol_th));
                g->AddDatumToInputSideData("countType_protocol_score", dni::Datum(countType_protocol_score));
                g->AddDatumToInputSideData("cond_countType_protocol_th", dni::Datum(cond_countType_protocol_th));

                // packet: countType_srcPortTcp
                g->AddDatumToInputStream("countType_srcPortTcp", dni::Datum(countType_srcPortTcp));
                g->AddDatumToInputStream("cond_countType_srcPortTcp", dni::Datum(cond_countType_srcPortTcp));
                g->AddDatumToInputSideData("countType_srcPortTcp_th", dni::Datum(countType_srcPortTcp_th));
                g->AddDatumToInputSideData("countType_srcPortTcp_score", dni::Datum(countType_srcPortTcp_score));
                g->AddDatumToInputSideData("cond_countType_srcPortTcp_th", dni::Datum(cond_countType_srcPortTcp_th));

                // packet: countType_dstPortTcp
                g->AddDatumToInputStream("countType_dstPortTcp", dni::Datum(countType_dstPortTcp));
                g->AddDatumToInputStream("cond_countType_dstPortTcp", dni::Datum(cond_countType_dstPortTcp));
                g->AddDatumToInputSideData("countType_dstPortTcp_th", dni::Datum(countType_dstPortTcp_th));
                g->AddDatumToInputSideData("countType_dstPortTcp_score", dni::Datum(countType_dstPortTcp_score));
                g->AddDatumToInputSideData("cond_countType_dstPortTcp_th", dni::Datum(cond_countType_dstPortTcp_th));

                // packet: countType_srcPortUdp
                g->AddDatumToInputStream("countType_srcPortUdp", dni::Datum(countType_srcPortUdp));
                g->AddDatumToInputStream("cond_countType_srcPortUdp", dni::Datum(cond_countType_srcPortUdp));
                g->AddDatumToInputSideData("countType_srcPortUdp_th", dni::Datum(countType_srcPortUdp_th));
                g->AddDatumToInputSideData("countType_srcPortUdp_score", dni::Datum(countType_srcPortUdp_score));
                g->AddDatumToInputSideData("cond_countType_srcPortUdp_th", dni::Datum(cond_countType_srcPortUdp_th));

                // packet: countType_dstPortUdp
                g->AddDatumToInputStream("countType_dstPortUdp", dni::Datum(countType_dstPortUdp));
                g->AddDatumToInputStream("cond_countType_dstPortUdp", dni::Datum(cond_countType_dstPortUdp));
                g->AddDatumToInputSideData("countType_dstPortUdp_th", dni::Datum(countType_dstPortUdp_th));
                g->AddDatumToInputSideData("countType_dstPortUdp_score", dni::Datum(countType_dstPortUdp_score));
                g->AddDatumToInputSideData("cond_countType_dstPortUdp_th", dni::Datum(cond_countType_dstPortUdp_th));


                // netdev: inMbps       
                g->AddDatumToInputStream("inMbps", dni::Datum(inMbps));
                g->AddDatumToInputSideData("inMbps_th", dni::Datum(inMbps_th));
                g->AddDatumToInputSideData("inMbps_score", dni::Datum(inMbps_score));

                // netdev: inKpps       
                g->AddDatumToInputStream("inKpps", dni::Datum(inKpps));
                g->AddDatumToInputSideData("inKpps_th", dni::Datum(inKpps_th));
                g->AddDatumToInputSideData("inKpps_score", dni::Datum(inKpps_score));

                // netdev: outMbps       
                g->AddDatumToInputStream("outMbps", dni::Datum(outMbps));
                g->AddDatumToInputSideData("outMbps_th", dni::Datum(outMbps_th));
                g->AddDatumToInputSideData("outMbps_score", dni::Datum(outMbps_score));

                // netdev: outKpps       
                g->AddDatumToInputStream("outKpps", dni::Datum(outKpps));
                g->AddDatumToInputSideData("outKpps_th", dni::Datum(outKpps_th));
                g->AddDatumToInputSideData("outKpps_score", dni::Datum(outKpps_score));


                // resource: cur_cpu     
                g->AddDatumToInputStream("cur_cpu", dni::Datum(cur_cpu));
                g->AddDatumToInputSideData("cur_cpu_th", dni::Datum(cur_cpu_th));
                g->AddDatumToInputSideData("cur_cpu_score", dni::Datum(cur_cpu_score));

                // resource: incr_mem_swap     
                g->AddDatumToInputStream("incr_mem_swap", dni::Datum(incr_mem_swap));
                g->AddDatumToInputSideData("incr_mem_swap_th", dni::Datum(incr_mem_swap_th));
                g->AddDatumToInputSideData("incr_mem_swap_score", dni::Datum(incr_mem_swap_score));

                // resource: incr_mem_virtual     
                g->AddDatumToInputStream("incr_mem_virtual", dni::Datum(incr_mem_virtual));
                g->AddDatumToInputSideData("incr_mem_virtual_th", dni::Datum(incr_mem_virtual_th));
                g->AddDatumToInputSideData("incr_mem_virtual_score", dni::Datum(incr_mem_virtual_score));

                // resource: incr_mem_full     
                g->AddDatumToInputStream("incr_mem_full", dni::Datum(incr_mem_full));
                g->AddDatumToInputSideData("incr_mem_full_th", dni::Datum(incr_mem_full_th));
                g->AddDatumToInputSideData("incr_mem_full_score", dni::Datum(incr_mem_full_score));

                // resource: incr_tcpconn_semi     
                g->AddDatumToInputStream("incr_tcpconn_semi", dni::Datum(incr_tcpconn_semi));
                g->AddDatumToInputSideData("incr_tcpconn_semi_th", dni::Datum(incr_tcpconn_semi_th));
                g->AddDatumToInputSideData("incr_tcpconn_semi_score", dni::Datum(incr_tcpconn_semi_score));

                // resource: incr_tcpconn_total     
                g->AddDatumToInputStream("incr_tcpconn_total", dni::Datum(incr_tcpconn_total));
                g->AddDatumToInputSideData("incr_tcpconn_total_th", dni::Datum(incr_tcpconn_total_th));
                g->AddDatumToInputSideData("incr_tcpconn_total_score", dni::Datum(incr_tcpconn_total_score));

                // packet: packet_abnormal_judge  
                // g->AddDatumToInputStream("packet_scores_sum", dni::Datum(packet_scores_sum));
                g->AddDatumToInputSideData("packet_scores_sum_th", dni::Datum(packet_scores_sum_th));
                g->AddDatumToInputSideData("packet_scores_sum_score", dni::Datum(packet_scores_sum_score));

                // netdev: netdev_abnormal_judge
                // g->AddDatumToInputStream("netdev_scores_sum", dni::Datum(netdev_scores_sum));
                g->AddDatumToInputSideData("netdev_scores_sum_th", dni::Datum(netdev_scores_sum_th));
                g->AddDatumToInputSideData("netdev_scores_sum_score", dni::Datum(netdev_scores_sum_score));

                // resource: resource_abnormal_judge
                // g->AddDatumToInputStream("resource_scores_sum", dni::Datum(resource_scores_sum));
                g->AddDatumToInputSideData("resource_scores_sum_th", dni::Datum(resource_scores_sum_th));
                g->AddDatumToInputSideData("resource_scores_sum_score", dni::Datum(resource_scores_sum_score));


                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        }
}

int main()
{
        spdlog::set_level(spdlog::level::trace);

        const std::string& proto = R"pb(
                type: "Anomaly Detection Graph"
                
                # packet: countTotal
                input_stream: "GIn_CountTotal:0:countTotal"
                input_side_data: "GIn_CountTotal_TH:0:countTotal_th"            
                input_side_data: "GIn_CountTotal_Score:0:countTotal_score"

                # packet: countType_length
                input_stream: "GIn_CountType_Length:0:countType_length"
                input_stream: "GCondIn_CountType_Length:0:cond_countType_length"
                input_side_data: "GIn_CountType_Length_TH:0:countType_length_th"            
                input_side_data: "GIn_CountType_Length_Score:0:countType_length_score"
                input_side_data: "GCondIn_CountType_Length_TH:0:cond_countType_length_th"

                # packet: countType_srcIP
                input_stream: "GIn_CountType_SrcIP:0:countType_srcIP"
                input_stream: "GCondIn_CountType_SrcIP:0:cond_countType_srcIP"
                input_side_data: "GIn_CountType_SrcIP_TH:0:countType_srcIP_th"            
                input_side_data: "GIn_CountType_SrcIP_Score:0:countType_srcIP_score"
                input_side_data: "GCondIn_CountType_SrcIP_TH:0:cond_countType_srcIP_th"

                # packet: countType_dstIP
                input_stream: "GIn_CountType_DstIP:0:countType_dstIP"
                input_stream: "GCondIn_CountType_DstIP:0:cond_countType_dstIP"
                input_side_data: "GIn_CountType_DstIP_TH:0:countType_dstIP_th"            
                input_side_data: "GIn_CountType_DstIP_Score:0:countType_dstIP_score"
                input_side_data: "GCondIn_CountType_DstIP_TH:0:cond_countType_dstIP_th"

                # packet: countType_protocol
                input_stream: "GIn_CountType_Protocol:0:countType_protocol"
                input_stream: "GCondIn_CountType_Protocol:0:cond_countType_protocol"
                input_side_data: "GIn_CountType_Protocol_TH:0:countType_protocol_th"            
                input_side_data: "GIn_CountType_Protocol_Score:0:countType_protocol_score"
                input_side_data: "GCondIn_CountType_Protocol_TH:0:cond_countType_protocol_th"

                # packet: countType_srcPortTcp
                input_stream: "GIn_CountType_SrcPortTcp:0:countType_srcPortTcp"
                input_stream: "GCondIn_CountType_SrcPortTcp:0:cond_countType_srcPortTcp"
                input_side_data: "GIn_CountType_SrcPortTcp_TH:0:countType_srcPortTcp_th"            
                input_side_data: "GIn_CountType_SrcPortTcp_Score:0:countType_srcPortTcp_score"
                input_side_data: "GCondIn_CountType_SrcPortTcp_TH:0:cond_countType_srcPortTcp_th"

                # packet: countType_dstPortTcp
                input_stream: "GIn_CountType_DstPortTcp:0:countType_dstPortTcp"
                input_stream: "GCondIn_CountType_DstPortTcp:0:cond_countType_dstPortTcp"
                input_side_data: "GIn_CountType_DstPortTcp_TH:0:countType_dstPortTcp_th"            
                input_side_data: "GIn_CountType_DstPortTcp_Score:0:countType_dstPortTcp_score"
                input_side_data: "GCondIn_CountType_DstPortTcp_TH:0:cond_countType_dstPortTcp_th"

                # packet: countType_srcPortUdp
                input_stream: "GIn_CountType_SrcPortUdp:0:countType_srcPortUdp"
                input_stream: "GCondIn_CountType_SrcPortUdp:0:cond_countType_srcPortUdp"
                input_side_data: "GIn_CountType_SrcPortUdp_TH:0:countType_srcPortUdp_th"            
                input_side_data: "GIn_CountType_SrcPortUdp_Score:0:countType_srcPortUdp_score"
                input_side_data: "GCondIn_CountType_SrcPortUdp_TH:0:cond_countType_srcPortUdp_th"

                # packet: countType_dstPortUdp
                input_stream: "GIn_CountType_DstPortUdp:0:countType_dstPortUdp"
                input_stream: "GCondIn_CountType_DstPortUdp:0:cond_countType_dstPortUdp"
                input_side_data: "GIn_CountType_DstPortUdp_TH:0:countType_dstPortUdp_th"            
                input_side_data: "GIn_CountType_DstPortUdp_Score:0:countType_dstPortUdp_score"
                input_side_data: "GCondIn_CountType_DstPortUdp_TH:0:cond_countType_dstPortUdp_th"


                # netdev: inMbps
                input_stream: "GIn_InMbps:0:inMbps"
                input_side_data: "GIn_InMbps_TH:0:inMbps_th"            
                input_side_data: "GIn_InMbps_Score:0:inMbps_score" 

                # netdev: inKpps
                input_stream: "GIn_InKpps:0:inKpps"
                input_side_data: "GIn_InKpps_TH:0:inKpps_th"            
                input_side_data: "GIn_InKpps_Score:0:inKpps_score" 

                # netdev: outMbps
                input_stream: "GIn_OutMbps:0:outMbps"
                input_side_data: "GIn_OutMbps_TH:0:outMbps_th"            
                input_side_data: "GIn_OutMbps_Score:0:outMbps_score" 

                # netdev: outKpps
                input_stream: "GIn_OutKpps:0:outKpps"
                input_side_data: "GIn_OutKpps_TH:0:outKpps_th"            
                input_side_data: "GIn_OutKpps_Score:0:outKpps_score" 


                # resource: cur_cpu
                input_stream: "GIn_Cur_Cpu:0:cur_cpu"
                input_side_data: "GIn_Cur_Cpu_TH:0:cur_cpu_th"            
                input_side_data: "GIn_Cur_Cpu_Score:0:cur_cpu_score"

                # resource: incr_mem_swap
                input_stream: "GIn_Incr_Mem_Swap:0:incr_mem_swap"
                input_side_data: "GIn_Incr_Mem_Swap_TH:0:incr_mem_swap_th"            
                input_side_data: "GIn_Incr_Mem_Swap_Score:0:incr_mem_swap_score"

                # resource: incr_mem_virtual
                input_stream: "GIn_Incr_Mem_Virtual:0:incr_mem_virtual"
                input_side_data: "GIn_Incr_Mem_Virtual_TH:0:incr_mem_virtual_th"            
                input_side_data: "GIn_Incr_Mem_Virtual_Score:0:incr_mem_virtual_score"

                # resource: incr_mem_full
                input_stream: "GIn_Incr_Mem_Full:0:incr_mem_full"
                input_side_data: "GIn_Incr_Mem_Full_TH:0:incr_mem_full_th"            
                input_side_data: "GIn_Incr_Mem_Full_Score:0:incr_mem_full_score"

                # resource: incr_tcpconn_semi
                input_stream: "GIn_Incr_Tcpconn_Semi:0:incr_tcpconn_semi"
                input_side_data: "GIn_Incr_Tcpconn_Semi_TH:0:incr_tcpconn_semi_th"            
                input_side_data: "GIn_Incr_Tcpconn_Semi_Score:0:incr_tcpconn_semi_score"

                # resource: incr_tcpconn_total
                input_stream: "GIn_Incr_Tcpconn_Total:0:incr_tcpconn_total"
                input_side_data: "GIn_Incr_Tcpconn_Total_TH:0:incr_tcpconn_total_th"            
                input_side_data: "GIn_Incr_Tcpconn_Total_Score:0:incr_tcpconn_total_score"

                # packet: packet_abnormal_judge
                # input_stream: "Packet_Scores_Sum:0:packet_scores_sum"
                input_side_data: "GIn_Packet_Scores_Sum_TH:0:packet_scores_sum_th"
                input_side_data: "GIn_Packet_Scores_Sum_Score:0:packet_scores_sum_score"

                # netdev: netdev_abnormal_judge
                # input_stream: "Netdev_Scores_Sum:0:netdev_scores_sum"
                input_side_data: "GIn_Netdev_Scores_Sum_TH:0:netdev_scores_sum_th"
                input_side_data: "GIn_Netdev_Scores_Sum_Score:0:netdev_scores_sum_score"

                # resource: resource_abnormal_judge
                # input_stream: "Resource_Scores_Sum:0:resource_scores_sum"
                input_side_data: "GIn_Resource_Scores_Sum_TH:0:resource_scores_sum_th"
                input_side_data: "GIn_Resource_Scores_Sum_Score:0:resource_scores_sum_score"


                # Graph output
                output_stream: "GOut_abnormal_res:0:abnormal_res"


                node{
                        name: "packet_countTotal_score"
                        task: "ThresholdTask"
                        input_stream: "GIn_CountTotal:0:countTotal"
                        input_side_data: "GIn_CountTotal_TH:0:countTotal_th"            
                        input_side_data: "GIn_CountTotal_Score:0:countTotal_score" 
                        output_stream: "NOut_packet_countTotal:0:packet_countTotal"
                }

                node{
                        name: "packet_countType_length_score"
                        task: "ConditionThresholdTask"
                        input_stream: "GIn_CountType_Length:0:countType_length"
                        input_stream: "GCondIn_CountType_Length:0:cond_countType_length"
                        input_side_data: "GIn_CountType_Length_TH:0:countType_length_th"            
                        input_side_data: "GIn_CountType_Length_Score:0:countType_length_score"
                        input_side_data: "GCondIn_CountType_Length_TH:0:cond_countType_length_th"
                        output_stream: "NOut_packet_countType_length:0:packet_countType_length" 
                }

                node{
                        name: "packet_countType_srcIP_score"
                        task: "ConditionThresholdTask"
                        input_stream: "GIn_CountType_SrcIP:0:countType_srcIP"
                        input_stream: "GCondIn_CountType_SrcIP:0:cond_countType_srcIP"
                        input_side_data: "GIn_CountType_SrcIP_TH:0:countType_srcIP_th"            
                        input_side_data: "GIn_CountType_SrcIP_Score:0:countType_srcIP_score"
                        input_side_data: "GCondIn_CountType_SrcIP_TH:0:cond_countType_srcIP_th"
                        output_stream: "NOut_packet_countType_srcIP:0:packet_countType_srcIP" 
                        } 

                node{
                        name: "packet_countType_dstIP_score"
                        task: "ConditionThresholdTask"
                        input_stream: "GIn_CountType_DstIP:0:countType_dstIP"
                        input_stream: "GCondIn_CountType_DstIP:0:cond_countType_dstIP"
                        input_side_data: "GIn_CountType_DstIP_TH:0:countType_dstIP_th"            
                        input_side_data: "GIn_CountType_DstIP_Score:0:countType_dstIP_score"
                        input_side_data: "GCondIn_CountType_DstIP_TH:0:cond_countType_dstIP_th"
                        output_stream: "NOut_packet_countType_dstIP:0:packet_countType_dstIP" 
                }

                node{
                        name: "packet_countType_protocol_score"
                        task: "ConditionThresholdTask"
                        input_stream: "GIn_CountType_Protocol:0:countType_protocol"
                        input_stream: "GCondIn_CountType_Protocol:0:cond_countType_protocol"
                        input_side_data: "GIn_CountType_Protocol_TH:0:countType_protocol_th"            
                        input_side_data: "GIn_CountType_Protocol_Score:0:countType_protocol_score"
                        input_side_data: "GCondIn_CountType_Protocol_TH:0:cond_countType_protocol_th"
                        output_stream: "NOut_packet_countType_protocol:0:packet_countType_protocol" 
                }

                node{
                        name: "packet_countType_srcPortTcp_score"
                        task: "ConditionThresholdTask"
                        input_stream: "GIn_CountType_SrcPortTcp:0:countType_srcPortTcp"
                        input_stream: "GCondIn_CountType_SrcPortTcp:0:cond_countType_srcPortTcp"
                        input_side_data: "GIn_CountType_SrcPortTcp_TH:0:countType_srcPortTcp_th"            
                        input_side_data: "GIn_CountType_SrcPortTcp_Score:0:countType_srcPortTcp_score"
                        input_side_data: "GCondIn_CountType_SrcPortTcp_TH:0:cond_countType_srcPortTcp_th"
                        output_stream: "NOut_packet_countType_srcPortTcp:0:packet_countType_srcPortTcp" 
                } 

                node{
                        name: "packet_countType_dstPortTcp_score"
                        task: "ConditionThresholdTask"
                        input_stream: "GIn_CountType_DstPortTcp:0:countType_dstPortTcp"
                        input_stream: "GCondIn_CountType_DstPortTcp:0:cond_countType_dstPortTcp"
                        input_side_data: "GIn_CountType_DstPortTcp_TH:0:countType_dstPortTcp_th"            
                        input_side_data: "GIn_CountType_DstPortTcp_Score:0:countType_dstPortTcp_score"
                        input_side_data: "GCondIn_CountType_DstPortTcp_TH:0:cond_countType_dstPortTcp_th"
                        output_stream: "NOut_packet_countType_dstPortTcp:0:packet_countType_dstPortTcp" 
                }  

                node{
                        name: "packet_countType_srcPortUdp_score"
                        task: "ConditionThresholdTask"
                        input_stream: "GIn_CountType_SrcPortUdp:0:countType_srcPortUdp"
                        input_stream: "GCondIn_CountType_SrcPortUdp:0:cond_countType_srcPortUdp"
                        input_side_data: "GIn_CountType_SrcPortUdp_TH:0:countType_srcPortUdp_th"            
                        input_side_data: "GIn_CountType_SrcPortUdp_Score:0:countType_srcPortUdp_score"
                        input_side_data: "GCondIn_CountType_SrcPortUdp_TH:0:cond_countType_srcPortUdp_th"
                        output_stream: "NOut_packet_countType_srcPortUdp:0:packet_countType_srcPortUdp" 
                }  

                node{
                        name: "packet_countType_dstPortUdp_score"
                        task: "ConditionThresholdTask"
                        input_stream: "GIn_CountType_DstPortUdp:0:countType_dstPortUdp"
                        input_stream: "GCondIn_CountType_DstPortUdp:0:cond_countType_dstPortUdp"
                        input_side_data: "GIn_CountType_DstPortUdp_TH:0:countType_dstPortUdp_th"            
                        input_side_data: "GIn_CountType_DstPortUdp_Score:0:countType_dstPortUdp_score"
                        input_side_data: "GCondIn_CountType_DstPortUdp_TH:0:cond_countType_dstPortUdp_th"
                        output_stream: "NOut_packet_countType_dstPortUdp:0:packet_countType_dstPortUdp" 
                }  

                node{
                        name: "packet_total_score"
                        task: "SumTask"
                        input_stream: "NOut_packet_countTotal:0:packet_countTotal"
                        input_stream: "NOut_packet_countType_length:0:packet_countType_length"
                        input_stream: "NOut_packet_countType_srcIP:0:packet_countType_srcIP" 
                        input_stream: "NOut_packet_countType_dstIP:0:packet_countType_dstIP"
                        input_stream: "NOut_packet_countType_protocol:0:packet_countType_protocol"
                        input_stream: "NOut_packet_countType_srcPortTcp:0:packet_countType_srcPortTcp" 
                        input_stream: "NOut_packet_countType_dstPortTcp:0:packet_countType_dstPortTcp"
                        input_stream: "NOut_packet_countType_srcPortUdp:0:packet_countType_srcPortUdp" 
                        input_stream: "NOut_packet_countType_dstPortUdp:0:packet_countType_dstPortUdp"
                        output_stream: "Packet_Scores_Sum:0:packet_scores_sum"
                }

                node{
                        name: "packet_abnormal_judge"
                        task: "ThresholdTask"
                        input_stream: "Packet_Scores_Sum:0:packet_scores_sum"
                        input_side_data: "GIn_Packet_Scores_Sum_TH:0:packet_scores_sum_th"
                        input_side_data: "GIn_Packet_Scores_Sum_Score:0:packet_scores_sum_score"
                        output_stream: "NOut_packet_abnormal:0:packet_abnormal"
                }

                node{
                        name: "netdev_inMbps_score"
                        task: "ThresholdTask"
                        input_stream: "GIn_InMbps:0:inMbps"
                        input_side_data: "GIn_InMbps_TH:0:inMbps_th"            
                        input_side_data: "GIn_InMbps_Score:0:inMbps_score" 
                        output_stream: "NOut_netdev_inMbps:0:netdev_inMbps"
                } 

                node{
                        name: "netdev_inKpps_score"
                        task: "ThresholdTask"
                        input_stream: "GIn_InKpps:0:inKpps"
                        input_side_data: "GIn_InKpps_TH:0:inKpps_th"            
                        input_side_data: "GIn_InKpps_Score:0:inKpps_score" 
                        output_stream: "NOut_netdev_inKpps:0:netdev_inKpps"
                } 

                node{
                        name: "netdev_outMbps_score"
                        task: "ThresholdTask"
                        input_stream: "GIn_OutMbps:0:outMbps"
                        input_side_data: "GIn_OutMbps_TH:0:outMbps_th"            
                        input_side_data: "GIn_OutMbps_Score:0:outMbps_score" 
                        output_stream: "NOut_netdev_outMbps:0:netdev_outMbps"
                }

                node{
                        name: "netdev_outKpps_score"
                        task: "ThresholdTask"
                        input_stream: "GIn_OutKpps:0:outKpps"
                        input_side_data: "GIn_OutKpps_TH:0:outKpps_th"            
                        input_side_data: "GIn_OutKpps_Score:0:outKpps_score" 
                        output_stream: "NOut_netdev_outKpps:0:netdev_outKpps"
                } 

                node{
                        name: "netdev_total_score"
                        task: "SumTask"
                        input_stream: "NOut_netdev_inMbps:0:netdev_inMbps"
                        input_stream: "NOut_netdev_inKpps:0:netdev_inKpps"
                        input_stream: "NOut_netdev_outMbps:0:netdev_outMbps"
                        input_stream: "NOut_netdev_outKpps:0:netdev_outKpps"
                        output_stream: "Netdev_Scores_Sum:0:netdev_scores_sum"
                }

                node{
                        name: "netdev_abnormal_judge"
                        task: "ThresholdTask"
                        input_stream: "Netdev_Scores_Sum:0:netdev_scores_sum"
                        input_side_data: "GIn_Netdev_Scores_Sum_TH:0:netdev_scores_sum_th"
                        input_side_data: "GIn_Netdev_Scores_Sum_Score:0:netdev_scores_sum_score"
                        output_stream: "NOut_netdev_abnormal:0:netdev_abnormal"
                }

                node{
                        name: "resource_cur_cpu_score"
                        task: "ThresholdTask"
                        input_stream: "GIn_Cur_Cpu:0:cur_cpu"
                        input_side_data: "GIn_Cur_Cpu_TH:0:cur_cpu_th"            
                        input_side_data: "GIn_Cur_Cpu_Score:0:cur_cpu_score" 
                        output_stream: "NOut_resource_cur_cpu:0:resource_cur_cpu"
                } 

                node{
                        name: "resource_incr_mem_swap_score"
                        task: "ThresholdTask"
                        input_stream: "GIn_Incr_Mem_Swap:0:incr_mem_swap"
                        input_side_data: "GIn_Incr_Mem_Swap_TH:0:incr_mem_swap_th"            
                        input_side_data: "GIn_Incr_Mem_Swap_Score:0:incr_mem_swap_score"
                        output_stream: "NOut_resource_incr_mem_swap:0:resource_incr_mem_swap"
                }  

                node{
                        name: "resource_incr_mem_virtual_score"
                        task: "ThresholdTask"
                        input_stream: "GIn_Incr_Mem_Virtual:0:incr_mem_virtual"
                        input_side_data: "GIn_Incr_Mem_Virtual_TH:0:incr_mem_virtual_th"            
                        input_side_data: "GIn_Incr_Mem_Virtual_Score:0:incr_mem_virtual_score"
                        output_stream: "NOut_resource_incr_mem_virtual:0:resource_incr_mem_virtual"
                } 

                node{
                        name: "resource_incr_mem_full_score"
                        task: "ThresholdTask"
                        input_stream: "GIn_Incr_Mem_Full:0:incr_mem_full"
                        input_side_data: "GIn_Incr_Mem_Full_TH:0:incr_mem_full_th"            
                        input_side_data: "GIn_Incr_Mem_Full_Score:0:incr_mem_full_score"
                        output_stream: "NOut_resource_incr_mem_full:0:resource_incr_mem_full"
                }  

                node{
                        name: "resource_incr_tcpconn_semi_score"
                        task: "ThresholdTask"
                        input_stream: "GIn_Incr_Tcpconn_Semi:0:incr_tcpconn_semi"
                        input_side_data: "GIn_Incr_Tcpconn_Semi_TH:0:incr_tcpconn_semi_th"            
                        input_side_data: "GIn_Incr_Tcpconn_Semi_Score:0:incr_tcpconn_semi_score"
                        output_stream: "NOut_resource_incr_tcpconn_semi:0:resource_incr_tcpconn_semi"
                }  

                node{
                        name: "resource_incr_tcpconn_total_score"
                        task: "ThresholdTask"
                        input_stream: "GIn_Incr_Tcpconn_Total:0:incr_tcpconn_total"
                        input_side_data: "GIn_Incr_Tcpconn_Total_TH:0:incr_tcpconn_total_th"            
                        input_side_data: "GIn_Incr_Tcpconn_Total_Score:0:incr_tcpconn_total_score"
                        output_stream: "NOut_resource_incr_tcpconn_total:0:resource_incr_tcpconn_total"
                }

                node{
                        name: "resource_total_score"
                        task: "SumTask"
                        input_stream: "NOut_resource_cur_cpu:0:resource_cur_cpu"
                        input_stream: "NOut_resource_incr_mem_swap:0:resource_incr_mem_swap"
                        input_stream: "NOut_resource_incr_mem_virtual:0:resource_incr_mem_virtual"
                        input_stream: "NOut_resource_incr_mem_full:0:resource_incr_mem_full"
                        input_stream: "NOut_resource_incr_tcpconn_semi:0:resource_incr_tcpconn_semi"
                        input_stream: "NOut_resource_incr_tcpconn_total:0:resource_incr_tcpconn_total"
                        output_stream: "Resource_Scores_Sum:0:resource_scores_sum"
                }

                node{
                        name: "resource_abnormal_judge"
                        task: "ThresholdTask"
                        input_stream: "Resource_Scores_Sum:0:resource_scores_sum"
                        input_side_data: "GIn_Resource_Scores_Sum_TH:0:resource_scores_sum_th"
                        input_side_data: "GIn_Resource_Scores_Sum_Score:0:resource_scores_sum_score"
                        output_stream: "NOut_resource_abnormal:0:resource_abnormal"
                }

                node{
                        name: "abnormal_judge_result"
                        task: "SndAdTask"
                        input_stream: "NOut_packet_abnormal:0:packet_abnormal"
                        input_stream: "NOut_netdev_abnormal:0:netdev_abnormal"
                        input_stream: "NOut_resource_abnormal:0:resource_abnormal"
                        output_stream: "GOut_abnormal_res:0:abnormal_res"
                }


        )pb";

        auto gc = dni::ParseStringToGraphConfig(proto);
        if (!gc)
        {
                spdlog::error("invalid pbtxt config: {}", proto);
                return -1;
        }
        spdlog::debug(
            "GraphConfig: {}, node size: {}", gc.value().type(), gc.value().node_size());

        dni::Graph* g = new dni::Graph(gc.value());

        std::string out00 = "packet_scores_sum";
        spdlog::debug("Create ObserveOutputStream: {}", out00);
        g->ObserveOutputStream(out00);

        std::string out01 = "netdev_scores_sum";
        spdlog::debug("Create ObserveOutputStream: {}", out01);
        g->ObserveOutputStream(out01);
        
        std::string out02 = "resource_scores_sum";
        spdlog::debug("Create ObserveOutputStream: {}", out02);
        g->ObserveOutputStream(out02);

        std::string out10 = "packet_abnormal";
        spdlog::debug("Create ObserveOutputStream: {}", out10);
        g->ObserveOutputStream(out10);

        std::string out11 = "netdev_abnormal";
        spdlog::debug("Create ObserveOutputStream: {}", out11);
        g->ObserveOutputStream(out11);

        std::string out12 = "resource_abnormal";
        spdlog::debug("Create ObserveOutputStream: {}", out12);
        g->ObserveOutputStream(out12);

        std::string out2 = "abnormal_res";
        spdlog::debug("Create ObserveOutputStream: {}", out2);
        g->ObserveOutputStream(out2);   

        g->PrepareForRun();
        inject_after(g, 0, 1, 0);
        g->RunOnce();
        g->Wait();

        auto ret00 = g->GetResult<double_t>(out00);
        spdlog::info("Gout {} result is: {}", out00, ret00);

        auto ret01 = g->GetResult<double_t>(out01);
        spdlog::info("Gout {} result is: {}", out01, ret01);

        auto ret02 = g->GetResult<double_t>(out02);
        spdlog::info("Gout {} result is: {}", out02, ret02);

        auto ret10 = g->GetResult<double_t>(out10);
        spdlog::info("Gout {} result is: {}", out10, ret10);

        auto ret11 = g->GetResult<double_t>(out11);
        spdlog::info("Gout {} result is: {}", out11, ret11);

        auto ret12 = g->GetResult<double_t>(out12);
        spdlog::info("Gout {} result is: {}", out12, ret12);


        auto ret2 = g->GetResult<int>(out2);
        spdlog::info("Gout {} result is: {}", out2, ret2);


        g->Finish();

        spdlog::info("main over");

        return 0;
}
