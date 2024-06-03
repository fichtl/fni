package test

import (
	"fmt"
	"testing"
	"time"

	graph "github.com/amianetworks/dni/sdk/graph"
)

func TestThresholdTask(t *testing.T) {
	graph_path := "/home/yf/workspace/dni/dni/go/sdk/test/threshold.yaml"
	//init g
	g, err := graph.InitialGraph(graph_path)
	if err != nil {
		t.Fatalf("failed to run g:%v", err)
	}
	g.PrepareForRun(make(map[string]interface{}))
	g.Run()
	//add inputs
	for i := 3; i < 5; i++ {
		g.AddGraphInputData(nil, "A")
		//get output
		// d, err := g.GetGraphOutputData("B")
		outputs, err := g.GetAllGraphOutputData()
		if err != nil {
			t.Log(err)
		}
		fmt.Println("Score:", outputs)
	}
	//destroy g
	err = g.Destroy()
	if err != nil {
		fmt.Println(err)
	}
}

func TestCondThresholdTask(t *testing.T) {
	graph_path := "condthreshold.yaml"
	//init g
	g, err := graph.InitialGraph(graph_path)
	if err != nil {
		t.Fatalf("failed to run g:%v", err)
	}
	g.PrepareForRun(make(map[string]interface{}))
	g.Run()
	//add inputs
	for i := 0; i < 1; i++ {
		for j := 1999; j < 2000; j++ {
			g.AddGraphInputData(float64(i), "stat")
			g.AddGraphInputData([]float64{float64(i * 200)}, "cond")
			//get output
			d, err := g.GetGraphOutputData("score")
			if err != nil {
				t.Log(err)
			}
			fmt.Println("score:", d)
		}
	}
	//destroy g
	err = g.Destroy()
	if err != nil {
		fmt.Println(err)
	}
}

func TestSndAdTask(t *testing.T) {
	graph_path := "abnormaljudge.yaml"
	//register executors

	//init g
	g, err := graph.InitialGraph(graph_path)
	if err != nil {
		t.Fatalf("failed to run g:%v", err)
	}
	g.PrepareForRun(make(map[string]interface{}))
	g.Run()
	//add inputs
	for i := 0; i < 2; i++ {
		for j := 0; j < 2; j++ {
			for z := 0; z < 2; z++ {
				g.AddGraphInputData(float64(i), "packet")
				g.AddGraphInputData(float64(j), "netdev")
				g.AddGraphInputData(float64(z), "resource")
				//get output
				d, err := g.GetGraphOutputData("abnormal_res")
				if err != nil {
					t.Log(err)
				}
				fmt.Println("Abnormal:", d)
			}
		}
	}
	//destroy g
	err = g.Destroy()
	if err != nil {
		fmt.Println(err)
	}
}

func TestSndPcapParseTask(t *testing.T) {
	graph_path := "pcap_parse.yaml"
	//register executors

	//init g
	g, err := graph.InitialGraph(graph_path)
	if err != nil {
		t.Fatalf("failed to run g:%v", err)
	}
	g.PrepareForRun(make(map[string]interface{}))
	g.Run()
	//add inputs
	file_path := "/home/yf/workspace/pcap/hping3-icmp.pcap"
	g.AddGraphInputData(file_path, "path")
	_, err = g.GetGraphOutputData("pinfos")
	if err != nil {
		t.Log(err)
	}
	//destroy g
	err = g.Destroy()
	if err != nil {
		fmt.Println(err)
	}
}

func TestFeatureCounterTask(t *testing.T) {
	graph_path := "pcap_feature.yaml"
	//register executors

	//init g
	g, err := graph.InitialGraph(graph_path)
	if err != nil {
		t.Fatalf("failed to run g:%v", err)
	}
	g.PrepareForRun(make(map[string]interface{}))
	g.Run()
	//add inputs
	file_path := "/home/yf/workspace/pcap/hping3-icmp.pcap"
	g.AddGraphInputData(file_path, "path")
	_, err = g.GetGraphOutputData("sip_count")
	if err != nil {
		t.Log(err)
	}
	time.Sleep(2 * time.Second)
	//destroy g
	err = g.Destroy()
	if err != nil {
		fmt.Println(err)
	}
}

func TestSndNumberStatsTask(t *testing.T) {
	graph_path := "num_feature.yaml"
	//register executors

	//init g
	g, err := graph.InitialGraph(graph_path)
	if err != nil {
		t.Fatalf("failed to run g:%v", err)
	}
	g.PrepareForRun(make(map[string]interface{}))
	g.Run()
	//add inputs
	file_path := "/home/yf/workspace/pcap/pktgen-cidr6.pcap"
	g.AddGraphInputData(file_path, "path")
	_, err = g.GetGraphOutputData("sipscore")
	if err != nil {
		t.Log(err)
	}
	time.Sleep(2 * time.Second)
	//destroy g
	err = g.Destroy()
	if err != nil {
		fmt.Println(err)
	}
}

func TestSndProtoStatsTask(t *testing.T) {
	graph_path := "proto_feature.yaml"
	//register executors

	//init g
	g, err := graph.InitialGraph(graph_path)
	if err != nil {
		t.Fatalf("failed to run g:%v", err)
	}
	g.PrepareForRun(make(map[string]interface{}))
	g.Run()
	//add inputs
	file_path := "/home/yf/workspace/pcap/pktgen-cidr6.pcap"
	g.AddGraphInputData(file_path, "path")
	_, err = g.GetGraphOutputData("protoscore")
	if err != nil {
		t.Log(err)
	}
	time.Sleep(2 * time.Second)
	//destroy g
	err = g.Destroy()
	if err != nil {
		fmt.Println(err)
	}
}

func TestMaxTask(t *testing.T) {
	graph_path := "num_feature_max.yaml"

	//init g
	g, err := graph.InitialGraph(graph_path)
	if err != nil {
		t.Fatalf("failed to run g:%v", err)
	}
	g.PrepareForRun(make(map[string]interface{}))
	g.Run()
	//add inputs
	file_path := "/home/yf/workspace/pcap/pktgen-cidr6.pcap"
	g.AddGraphInputData(file_path, "path")
	_, err = g.GetGraphOutputData("maxsocre")
	if err != nil {
		t.Log(err)
	}
	time.Sleep(2 * time.Second)
	//destroy g
	err = g.Destroy()
	if err != nil {
		fmt.Println(err)
	}
}

func TestIPMergeTask(t *testing.T) {
	graph_path := "ip_merge.yaml"
	//register executors

	//init g
	g, err := graph.InitialGraph(graph_path)
	if err != nil {
		t.Fatalf("failed to run g:%v", err)
	}
	g.PrepareForRun(make(map[string]interface{}))
	g.Run()
	//add inputs
	file_path := "/home/yf/workspace/pcap/pktgen-cidr28.pcap"
	g.AddGraphInputData(file_path, "path")
	normal_ips := make(map[uint32]struct{})
	g.AddGraphInputData(normal_ips, "knownips")
	_, err = g.GetGraphOutputData("attakerIPMergeRes")
	if err != nil {
		t.Log(err)
	}
	time.Sleep(2 * time.Second)
	//destroy g
	err = g.Destroy()
	if err != nil {
		fmt.Println(err)
	}
}

func TestSIPBaseMerge(t *testing.T) {
	graph_path := "sip_base_merge.yaml"
	//init g
	g, err := graph.InitialGraph(graph_path)
	if err != nil {
		t.Fatalf("failed to run g:%v", err)
	}
	g.PrepareForRun(make(map[string]interface{}))
	g.Run()
	//add inputs
	file_path := "/home/yf/workspace/pcap/pktgen-cidr28.pcap"
	g.AddGraphInputData(file_path, "path")
	normal_ips := make(map[uint32]struct{})
	g.AddGraphInputData(normal_ips, "knownips")
	g.AddGraphInputData("ens1f1", "nic")
	nicrecord, err := g.GetGraphOutputData("dms_rules_prepare")
	if err != nil {
		t.Log(err)
	}
	t.Log(nicrecord)
	time.Sleep(5 * time.Second)
	//destroy g
	err = g.Destroy()
	if err != nil {
		fmt.Println(err)
	}
}

func TestGenDMSRule(t *testing.T) {
	graph_path := "gen_dms_rules.yaml"
	//init g
	g, err := graph.InitialGraph(graph_path)
	if err != nil {
		t.Fatalf("failed to run g:%v", err)
	}
	g.PrepareForRun(make(map[string]interface{}))
	g.Run()
	//add inputs
	file_path := "/home/yf/workspace/pcap/pktgen-cidr28.pcap"
	g.AddGraphInputData(file_path, "path")
	normal_ips := make(map[uint32]struct{})
	g.AddGraphInputData(normal_ips, "knownips")
	g.AddGraphInputData("ens1f1", "nic")
	g.AddGraphInputData([]float64{200, 0.6, 300, 0.4}, "netdev")
	nicrecord, err := g.GetGraphOutputData("dmsrules")
	if err != nil {
		t.Log(err)
	}
	t.Log(nicrecord)
	time.Sleep(2 * time.Second)
}

func TestNetRecordMerge(t *testing.T) {
	graph_path := "net_record_merge.yaml"
	//init g
	g, err := graph.InitialGraph(graph_path)
	if err != nil {
		t.Fatalf("failed to run g:%v", err)
	}
	g.PrepareForRun(make(map[string]interface{}))
	g.Run()
	//add inputs
	file_path := "/home/yf/workspace/pcap/pktgen-cidr28.pcap"
	g.AddGraphInputData(file_path, "path")
	normal_ips := make(map[uint32]struct{})
	g.AddGraphInputData(normal_ips, "knownips")
	g.AddGraphInputData("ens1f1", "nic")
	nicrecord, err := g.GetGraphOutputData("iplink")
	if err != nil {
		t.Log(err)
	}
	t.Log(nicrecord)
	time.Sleep(2 * time.Second)
}
