package test

import (
	"fmt"
	"log"
	"testing"
	"time"

	graph "github.com/amianetworks/dni/sdk/graph"
	_ "github.com/amianetworks/dni/sdk/task/dms/abitrator"
	"github.com/amianetworks/dni/sdk/task/dms/analyzer"
	"github.com/amianetworks/dni/sdk/task/dms/assessor"
	"github.com/amianetworks/dni/service/generator"
	"github.com/google/gopacket"
)

func TestDmsAssessTask(t *testing.T) {
	graph_path := "./config/dms/dms_assess_task.yaml"
	//init g
	g, err := graph.InitialGraph(graph_path)
	if err != nil {
		t.Fatalf("failed to run g:%v", err)
	}
	sidedataMap := map[string]interface{}{}
	g.PrepareForRun(sidedataMap)
	g.Run()
	assessData := assessor.AssessorData{}
	assessData.NicSpeed = map[string]uint64{"ens1f1np1": 25000}
	assessData.BW = map[string]map[string]uint64{"ens1f1np1": make(map[string]uint64)}
	assessData.SNMP = make(map[string]uint64)
	assessData.TCPConn = make(map[string]uint64)
	g.AddGraphInputData(assessData, "assessorData")
	assessInd, err := g.GetGraphOutputData("assessorInd")
	if err != nil {
		t.Log(err)
	}
	t.Log(assessInd)
	time.Sleep(1 * time.Second)
	//destroy g
	err = g.Destroy()
	if err != nil {
		fmt.Println(err)
	}
}

func TestAbitrateTask1(t *testing.T) {
	graph_path := "./config/dms/dms_abitrate_task-01.yaml"
	//init g
	g, err := graph.InitialGraph(graph_path)
	if err != nil {
		t.Fatalf("failed to run g:%v", err)
	}
	g.PrepareForRun(make(map[string]interface{}))
	g.Run()
	for i := 0; i < 1; i++ {
		assessInd := &assessor.AssessorInd{
			CPU:      assessor.CPU_HIGH,
			BW:       assessor.BW_RX_BPS_HIGH,
			PerNicBW: map[string]map[string]bool{"eno1": {"rx_bytes": true}},
			TCPConn:  assessor.TCPCONN_ESTABLISHED,
			SNMP:     assessor.PROTO_TCP_SYN,
		}
		g.AddGraphInputData(assessInd, "assessorInd")
		query, err := g.GetGraphOutputData("query")
		if err != nil {
			t.Log(err)
		}
		t.Log(query)
		intelli, err := g.GetGraphOutputData("intelli")
		if err != nil {
			t.Log(err)
		}
		t.Log(intelli)
	}
	//destroy g
	err = g.Destroy()
	if err != nil {
		fmt.Println(err)
	}
}
func TestConnDiagTask(t *testing.T) {
	graph_path := "./config/dms/dms_conn_diag_task-01.yaml"
	//init g
	g, err := graph.InitialGraph(graph_path)
	if err != nil {
		t.Fatalf("failed to run g:%v", err)
	}
	g.PrepareForRun(make(map[string]interface{}))
	g.Run()
	conns := generator.GetTcpConnInfos()
	for i := 0; i < 5; i++ {
		assessInd := &assessor.AssessorInd{
			CPU:      assessor.CPU_HIGH,
			BW:       assessor.BW_RX_BPS_HIGH,
			PerNicBW: map[string]map[string]bool{"eno1": {"rx_bytes": true}},
			TCPConn:  assessor.TCPCONN_ESTABLISHED,
			SNMP:     assessor.PROTO_TCP_SYN,
		}
		g.AddGraphInputData(assessInd, "assessorInd")
		g.AddGraphInputData(conns, "tcpconn")
		conndiag, err := g.GetGraphOutputData("conndiag")
		if err != nil {
			t.Log(err)
		}
		t.Log(conndiag.(*analyzer.ConnDiag).Estab)
	}
	//destroy g
	err = g.Destroy()
	if err != nil {
		fmt.Println(err)
	}
}

func TestProtoDiagTask(t *testing.T) {
	graph_path := "./config/dms/dms_proto_diag_task.yaml"
	//init g
	g, err := graph.InitialGraph(graph_path)
	if err != nil {
		t.Fatalf("failed to run g:%v", err)
	}
	//nicip
	nicip := map[string]string{"wlp1s0": "192.168.1.53"}
	sidedataMap := make(map[string]interface{})
	g.PrepareForRun(sidedataMap)
	g.Run()
	//assessInd
	assessInd := &assessor.AssessorInd{
		CPU:      assessor.CPU_HIGH,
		BW:       assessor.BW_RX_BPS_HIGH,
		PerNicBW: map[string]map[string]bool{"wlp1s0": {"rx_bytes": true}},
		TCPConn:  assessor.TCPCONN_ESTABLISHED,
		SNMP:     assessor.PROTO_ICMP,
	}
	//ctinfos
	ctinfos, _ := generator.GetCtInfoConntrack()
	//listen ports
	listen := generator.GetListenInfos()
	//packet
	//packet
	packet := map[string][]gopacket.Packet{
		"wlp1s0": make([]gopacket.Packet, 0),
	}
	ps, _ := generator.GetPackets("/home/yf/Workspace/pcap/hping3-icmp.pcap")
	packet["wlp1s0"] = ps
	//nicip
	g.AddGraphInputData(assessInd, "assessorInd")
	g.AddGraphInputData(ctinfos, "conntrack")
	g.AddGraphInputData(listen, "listen")
	g.AddGraphInputData(packet, "packet")
	g.AddGraphInputData(nicip, "nicIP")
	ratios := map[string]float64{"wlp1s0": 0.01}
	g.AddGraphInputData(ratios, "ratio")
	protodiag, err := g.GetGraphOutputData("protodiag")
	if err != nil {
		t.Log(err)
	}
	log.Println("ProtoDiag:", protodiag)
	//destroy g
	err = g.Destroy()
	if err != nil {
		fmt.Println(err)
	}
}

func TestSlowDetectTask(t *testing.T) {
	graph_path := "./config/dms/dms_slow_detection_task.yaml"
	//init g
	g, err := graph.InitialGraph(graph_path)
	if err != nil {
		t.Fatalf("failed to run g:%v", err)
	}
	//nicdata
	sidedataMap := make(map[string]interface{})
	g.PrepareForRun(sidedataMap)
	g.Run()
	conns := generator.GetTcpConnInfos()
	assessInd := &assessor.AssessorInd{
		CPU:      assessor.CPU_HIGH,
		BW:       assessor.BW_RX_BPS_HIGH,
		PerNicBW: map[string]map[string]bool{"wlp1s0": {"rx_bytes": true}},
		TCPConn:  assessor.TCPCONN_ESTABLISHED,
		SNMP:     assessor.PROTO_TCP_SYN,
	}
	nicip := map[string]string{"wlp1s0": "192.168.1.53"}
	g.AddGraphInputData(assessInd, "assessorInd")
	g.AddGraphInputData(conns, "tcpconn")
	g.AddGraphInputData(nicip, "nicIP")
	respond, err := g.GetGraphOutputData("respond")
	if err != nil {
		t.Log(err)
	}
	t.Log("Slow attack info:", respond)
	//destroy g
	err = g.Destroy()
	if err != nil {
		fmt.Println(err)
	}
}

func TestFloodDetectTask(t *testing.T) {
	graph_path := "./config/dms/dms_flood_detect_task.yaml"
	//init g
	g, err := graph.InitialGraph(graph_path)
	if err != nil {
		t.Fatalf("failed to run g:%v", err)
	}
	//nicip
	sidedataMap := make(map[string]interface{})
	g.PrepareForRun(sidedataMap)
	g.Run()
	//assessInd
	assessInd := &assessor.AssessorInd{
		CPU:      assessor.CPU_HIGH,
		BW:       assessor.BW_RX_BPS_HIGH,
		PerNicBW: map[string]map[string]bool{"wlp1s0": {"rx_bytes": true}},
		TCPConn:  assessor.TCPCONN_ESTABLISHED,
		SNMP:     assessor.PROTO_ICMP,
	}
	//ctinfos
	ctinfos, _ := generator.GetCtInfoConntrack()
	//listen ports
	listen := generator.GetListenInfos()
	//packet
	packet := map[string][]gopacket.Packet{
		"wlp1s0": make([]gopacket.Packet, 0),
	}
	ps, _ := generator.GetPackets("/home/yf/Workspace/pcap/hping3-icmp.pcap")
	packet["wlp1s0"] = ps
	//ratio
	ratios := map[string]float64{"wlp1s0": 0.01}
	//nicIP
	nicip := map[string]string{"wlp1s0": "192.168.1.53"}
	//add data
	startTime := time.Now()
	g.AddGraphInputData(assessInd, "assessorInd")
	g.AddGraphInputData(ctinfos, "conntrack")
	g.AddGraphInputData(listen, "listen")
	g.AddGraphInputData(packet, "packet")
	g.AddGraphInputData(ratios, "ratio")
	g.AddGraphInputData(nicip, "nicIP")
	responds, err := g.GetGraphOutputData("responds")
	if err != nil {
		t.Log(err)
	}
	t.Log("Flood Attack Info:", responds)
	endTime := time.Now()
	duration := endTime.Sub(startTime)
	t.Log(duration)
	//destroy g
	err = g.Destroy()
	if err != nil {
		fmt.Println(err)
	}
}
