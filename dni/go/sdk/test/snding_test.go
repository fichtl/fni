package test

import (
	"fmt"
	"testing"
	"time"

	"github.com/amianetworks/dni/sdk/runner"
	"github.com/amianetworks/dni/sdk/scheduler"
)

func TestThresholdRunner(t *testing.T) {
	graph_path := "/home/yf/workspace/dni/dni/go/sdk/test/threshold.yaml"
	//register executors
	runner.InitExecutorRegistry()
	//init graph
	graph, err := scheduler.InitialGraph(graph_path)
	if err != nil {
		t.Fatalf("failed to run graph:%v", err)
	}
	graph.GetReady()
	graph.Run()
	//add inputs
	for i := 0; i < 50; i++ {
		graph.AddGraphInputData(float64(i), "A")
		//get output
		d, err := graph.GetGraphOutputData("B")
		if err != nil {
			t.Log(err)
		}
		fmt.Println("Score:", d)
	}
	//destroy graph
	err = graph.Destroy()
	if err != nil {
		fmt.Println(err)
	}
}

func TestCondThresholdRunner(t *testing.T) {
	graph_path := "condthreshold.yaml"
	//register executors
	runner.InitExecutorRegistry()
	//init graph
	graph, err := scheduler.InitialGraph(graph_path)
	if err != nil {
		t.Fatalf("failed to run graph:%v", err)
	}
	graph.GetReady()
	graph.Run()
	//add inputs
	for i := 0; i < 600; i++ {
		for j := 1999; j < 2001; j++ {
			graph.AddGraphInputData(float64(i), "Val")
			graph.AddGraphInputData(float64(i*200), "Condval")
			//get output
			d, err := graph.GetGraphOutputData("Score")
			if err != nil {
				t.Log(err)
			}
			fmt.Println("Score:", d)
		}
	}
	//destroy graph
	err = graph.Destroy()
	if err != nil {
		fmt.Println(err)
	}
}

func TestAbnormalJudge(t *testing.T) {
	graph_path := "abnormaljudge.yaml"
	//register executors
	runner.InitExecutorRegistry()
	//init graph
	graph, err := scheduler.InitialGraph(graph_path)
	if err != nil {
		t.Fatalf("failed to run graph:%v", err)
	}
	graph.GetReady()
	graph.Run()
	//add inputs
	for i := 0; i < 2; i++ {
		for j := 0; j < 2; j++ {
			for z := 0; z < 2; z++ {
				graph.AddGraphInputData(float64(i), "Pscore")
				graph.AddGraphInputData(float64(j), "Nscore")
				graph.AddGraphInputData(float64(z), "Rscore")
				//get output
				d, err := graph.GetGraphOutputData("Abnormal")
				if err != nil {
					t.Log(err)
				}
				fmt.Println("Abnormal:", d)
			}
		}
	}
	//destroy graph
	err = graph.Destroy()
	if err != nil {
		fmt.Println(err)
	}
}

func TestPcapParseExecutor(t *testing.T) {
	graph_path := "pcap_parse.yaml"
	//register executors
	runner.InitExecutorRegistry()
	//init graph
	graph, err := scheduler.InitialGraph(graph_path)
	if err != nil {
		t.Fatalf("failed to run graph:%v", err)
	}
	graph.GetReady()
	graph.Run()
	//add inputs
	file_path := "/home/yf/Workspace/dni/dni/go/test/test_sources/2024_0515_1406_17.324-enp5s0f1np1_ddos21.pcap"
	graph.AddGraphInputData(file_path, "Filepath")
	_, err = graph.GetGraphOutputData("PcapParseResult")
	if err != nil {
		t.Log(err)
	}
	//destroy graph
	err = graph.Destroy()
	if err != nil {
		fmt.Println(err)
	}
}

func TestPacketFeatureExecutor(t *testing.T) {
	graph_path := "pcap_feature.yaml"
	//register executors
	runner.InitExecutorRegistry()
	//init graph
	graph, err := scheduler.InitialGraph(graph_path)
	if err != nil {
		t.Fatalf("failed to run graph:%v", err)
	}
	graph.GetReady()
	graph.Run()
	//add inputs
	file_path := "/home/yf/Workspace/dni/dni/go/test/test_sources/2024_0515_1406_17.324-enp5s0f1np1_ddos21.pcap"
	graph.AddGraphInputData(file_path, "Filepath")
	_, err = graph.GetGraphOutputData("PcapParseResult")
	if err != nil {
		t.Log(err)
	}
	time.Sleep(2 * time.Second)
	//destroy graph
	err = graph.Destroy()
	if err != nil {
		fmt.Println(err)
	}
}

func TestNumberFeatureExecutor(t *testing.T) {
	graph_path := "num_feature.yaml"
	//register executors
	runner.InitExecutorRegistry()
	//init graph
	graph, err := scheduler.InitialGraph(graph_path)
	if err != nil {
		t.Fatalf("failed to run graph:%v", err)
	}
	graph.GetReady()
	graph.Run()
	//add inputs
	file_path := "/home/yf/Workspace/dni/dni/go/test/test_sources/2024_0515_1406_17.324-enp5s0f1np1_ddos21.pcap"
	graph.AddGraphInputData(file_path, "Filepath")
	_, err = graph.GetGraphOutputData("PcapParseResult")
	if err != nil {
		t.Log(err)
	}
	time.Sleep(2 * time.Second)
	//destroy graph
	err = graph.Destroy()
	if err != nil {
		fmt.Println(err)
	}
}

func TestProtoFeatureExecutor(t *testing.T) {
	graph_path := "proto_feature.yaml"
	//register executors
	runner.InitExecutorRegistry()
	//init graph
	graph, err := scheduler.InitialGraph(graph_path)
	if err != nil {
		t.Fatalf("failed to run graph:%v", err)
	}
	graph.GetReady()
	graph.Run()
	//add inputs
	file_path := "/home/yf/Workspace/dni/dni/go/test/test_sources/2024_0515_1406_17.324-enp5s0f1np1_ddos21.pcap"
	graph.AddGraphInputData(file_path, "Filepath")
	_, err = graph.GetGraphOutputData("PcapParseResult")
	if err != nil {
		t.Log(err)
	}
	time.Sleep(2 * time.Second)
	//destroy graph
	err = graph.Destroy()
	if err != nil {
		fmt.Println(err)
	}
}

func TestMaxExecutor(t *testing.T) {
	graph_path := "num_feature_max.yaml"
	//register executors
	runner.InitExecutorRegistry()
	//init graph
	graph, err := scheduler.InitialGraph(graph_path)
	if err != nil {
		t.Fatalf("failed to run graph:%v", err)
	}
	graph.GetReady()
	graph.Run()
	//add inputs
	file_path := "/home/yf/Workspace/dni/dni/go/test/test_sources/2024_0515_1406_17.324-enp5s0f1np1_ddos21.pcap"
	graph.AddGraphInputData(file_path, "Filepath")
	_, err = graph.GetGraphOutputData("MaxScore")
	if err != nil {
		t.Log(err)
	}
	time.Sleep(2 * time.Second)
	//destroy graph
	err = graph.Destroy()
	if err != nil {
		fmt.Println(err)
	}
}

func TestIPMergeExecutor(t *testing.T) {
	graph_path := "ip_merge.yaml"
	//register executors
	runner.InitExecutorRegistry()
	//init graph
	graph, err := scheduler.InitialGraph(graph_path)
	if err != nil {
		t.Fatalf("failed to run graph:%v", err)
	}
	graph.GetReady()
	graph.Run()
	//add inputs
	file_path := "/home/yf/workspace/pcap/pktgen-cidr28.pcap"
	graph.AddGraphInputData(file_path, "Filepath")
	normal_ips := make(map[uint32]struct{})
	graph.AddGraphInputData(normal_ips, "NormalIPs")
	_, err = graph.GetGraphOutputData("AttackIPNets")
	if err != nil {
		t.Log(err)
	}
	time.Sleep(2 * time.Second)
	//destroy graph
	err = graph.Destroy()
	if err != nil {
		fmt.Println(err)
	}
}

func TestSIPBaseMerge(t *testing.T) {
	graph_path := "sip_base_merge.yaml"
	//register executors
	runner.InitExecutorRegistry()
	//init graph
	graph, err := scheduler.InitialGraph(graph_path)
	if err != nil {
		t.Fatalf("failed to run graph:%v", err)
	}
	graph.GetReady()
	graph.Run()
	//add inputs
	file_path := "/home/yf/Workspace/dni/dni/go/test/test_sources/2024_0515_1406_17.324-enp5s0f1np1_ddos21.pcap"
	graph.AddGraphInputData(file_path, "Filepath")
	normal_ips := make(map[uint32]struct{})
	graph.AddGraphInputData(normal_ips, "NormalIPs")
	graph.AddGraphInputData("ens1f1", "HostNicSign")
	nicrecord, err := graph.GetGraphOutputData("NicRecords")
	if err != nil {
		t.Log(err)
	}
	t.Log(nicrecord)
	time.Sleep(2 * time.Second)
	//destroy graph
	err = graph.Destroy()
	if err != nil {
		fmt.Println(err)
	}
}

func TestGenDMSRule(t *testing.T) {
	graph_path := "gen_dms_rules.yaml"
	//register executors
	runner.InitExecutorRegistry()
	//init graph
	graph, err := scheduler.InitialGraph(graph_path)
	if err != nil {
		t.Fatalf("failed to run graph:%v", err)
	}
	graph.GetReady()
	graph.Run()
	//add inputs
	file_path := "/home/yf/Workspace/dni/dni/go/test/test_sources/2024_0515_1406_17.324-enp5s0f1np1_ddos21.pcap"
	graph.AddGraphInputData(file_path, "Filepath")
	normal_ips := make(map[uint32]struct{})
	graph.AddGraphInputData(normal_ips, "NormalIPs")
	graph.AddGraphInputData("ens1f1", "HostNicSign")
	graph.AddGraphInputData([]float64{200, 0.6, 300, 0.4}, "Netdev")
	nicrecord, err := graph.GetGraphOutputData("DMSRules")
	if err != nil {
		t.Log(err)
	}
	t.Log(nicrecord)
	time.Sleep(2 * time.Second)
}
