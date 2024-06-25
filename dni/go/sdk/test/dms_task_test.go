package test

import (
	"fmt"
	"testing"
	"time"

	graph "github.com/amianetworks/dni/sdk/graph"
	_ "github.com/amianetworks/dni/sdk/task/dms/abitrator"
	"github.com/amianetworks/dni/sdk/task/dms/analyzer"
	"github.com/amianetworks/dni/sdk/task/dms/assessor"
	dms "github.com/amianetworks/dni/sdk/task/dms/data"
	"github.com/google/gopacket"
)

func TestDMSAssessTask(t *testing.T) {
	graph_path := "/home/yf/Workspace/dni/dni/go/sdk/test/source/dms/dms_assess_task.yaml"
	//init g
	g, err := graph.InitialGraph(graph_path)
	if err != nil {
		t.Fatalf("failed to run g:%v", err)
	}
	//nicspeed
	nicspeed := map[string]uint64{"eno1": 1000}
	sidedataMap := map[string]interface{}{"nicSpeed": nicspeed}
	g.PrepareForRun(sidedataMap)
	g.Run()
	devs := []string{"eno1"}
	ag := dms.NewAssessorDataGenerator(devs)
	for i := 0; i < 5; i++ {
		assessData, err := ag.GetAssessData()
		if err != nil {
			t.Log(err)
			continue
		}
		t.Log(assessData)
		g.AddGraphInputData(assessData, "assessorData")
		assessInd, err := g.GetGraphOutputData("assessorInd")
		if err != nil {
			t.Log(err)
		}
		t.Log(assessInd)
		time.Sleep(1 * time.Second)
	}
	//destroy g
	err = g.Destroy()
	if err != nil {
		fmt.Println(err)
	}
}

func TestAbitrateTask1(t *testing.T) {
	graph_path := "/home/yf/Workspace/dni/dni/go/sdk/test/source/dms/dms_abitrate_task-01.yaml"
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

func TestAbitrateTask2(t *testing.T) {
	graph_path := "/home/yf/Workspace/dni/dni/go/sdk/test/source/dms/dms_abitrate_task-02.yaml"
	//init g
	g, err := graph.InitialGraph(graph_path)
	if err != nil {
		t.Fatalf("failed to run g:%v", err)
	}
	nicspeed := map[string]uint64{"eno1": 1000}
	sidedataMap := map[string]interface{}{"nicSpeed": nicspeed}
	g.PrepareForRun(sidedataMap)
	g.Run()
	devs := []string{"eno1"}
	ag := dms.NewAssessorDataGenerator(devs)
	for i := 0; i < 5; i++ {
		assessData, err := ag.GetAssessData()
		if err != nil {
			t.Log(err)
			continue
		}
		t.Log(assessData)
		g.AddGraphInputData(assessData, "assessorData")
		query, err := g.GetGraphOutputData("query")
		if err != nil {
			t.Log(err)
		}
		t.Log(query)
		time.Sleep(1 * time.Second)
	}
	//destroy g
	err = g.Destroy()
	if err != nil {
		fmt.Println(err)
	}
}

func TestConnDiagTask(t *testing.T) {
	graph_path := "./source/dms/dms_conn_diag_task-01.yaml"
	//init g
	g, err := graph.InitialGraph(graph_path)
	if err != nil {
		t.Fatalf("failed to run g:%v", err)
	}
	g.PrepareForRun(make(map[string]interface{}))
	g.Run()
	conns := dms.GetTcpConnInfos()
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
	graph_path := "./source/dms/dms_proto_diag_task.yaml"
	//init g
	g, err := graph.InitialGraph(graph_path)
	if err != nil {
		t.Fatalf("failed to run g:%v", err)
	}
	//nicip
	nicip := map[string]string{"wlp1s0": "192.168.1.53"}
	sidedataMap := make(map[string]interface{})
	sidedataMap["nicip"] = nicip
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
	ctinfos, _ := dms.GetCtInfoConntrack()
	//listen ports
	listen := dms.GetListenInfos()
	//packet
	//packet
	packet := map[string][]gopacket.Packet{
		"wlp1s0": make([]gopacket.Packet, 0),
	}
	ps, _ := dms.GetPackets("/home/yf/Workspace/pcap/hping3-icmp.pcap")
	packet["wlp1s0"] = ps
	//nicip
	g.AddGraphInputData(assessInd, "assessorInd")
	g.AddGraphInputData(ctinfos, "conntrack")
	g.AddGraphInputData(listen, "listen")
	g.AddGraphInputData(packet, "packet")
	ratios := map[string]float64{"wlp1s0": 0.01}
	g.AddGraphInputData(ratios, "ratio")
	protodiag, err := g.GetGraphOutputData("protodiag")
	if err != nil {
		t.Log(err)
	}
	t.Log(protodiag.(*analyzer.ProtoDiag).Stat["wlp1s0"])
	//destroy g
	err = g.Destroy()
	if err != nil {
		fmt.Println(err)
	}
}

func TestSlowDetectTask(t *testing.T) {
	graph_path := "./source/dms/dms_slow_detection_task.yaml"
	//init g
	g, err := graph.InitialGraph(graph_path)
	if err != nil {
		t.Fatalf("failed to run g:%v", err)
	}
	//nicdata
	nicip := map[string]string{"wlp1s0": "192.168.1.53"}
	sidedataMap := make(map[string]interface{})
	sidedataMap["nicip"] = nicip
	g.PrepareForRun(sidedataMap)
	g.Run()
	conns := dms.GetTcpConnInfos()
	for i := 0; i < 5; i++ {
		assessInd := &assessor.AssessorInd{
			CPU:      assessor.CPU_HIGH,
			BW:       assessor.BW_RX_BPS_HIGH,
			PerNicBW: map[string]map[string]bool{"wlp1s0": {"rx_bytes": true}},
			TCPConn:  assessor.TCPCONN_ESTABLISHED,
			SNMP:     assessor.PROTO_TCP_SYN,
		}
		g.AddGraphInputData(assessInd, "assessorInd")
		g.AddGraphInputData(conns, "tcpconn")
		respond, err := g.GetGraphOutputData("respond")
		if err != nil {
			t.Log(err)
		}
		t.Log(respond)
	}
	//destroy g
	err = g.Destroy()
	if err != nil {
		fmt.Println(err)
	}
}

func TestFloodDetectTask(t *testing.T) {
	graph_path := "./source/dms/dms_flood_detect_task.yaml"
	//init g
	g, err := graph.InitialGraph(graph_path)
	if err != nil {
		t.Fatalf("failed to run g:%v", err)
	}
	//nicip
	nicip := map[string]string{"wlp1s0": "192.168.1.53"}
	sidedataMap := make(map[string]interface{})
	sidedataMap["nicip"] = nicip
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
	ctinfos, _ := dms.GetCtInfoConntrack()
	//listen ports
	listen := dms.GetListenInfos()
	//packet
	packet := map[string][]gopacket.Packet{
		"wlp1s0": make([]gopacket.Packet, 0),
	}
	ps, _ := dms.GetPackets("/home/yf/Workspace/pcap/hping3-icmp.pcap")
	packet["wlp1s0"] = ps
	//ratio
	ratios := map[string]float64{"wlp1s0": 0.01}
	//add data
	startTime := time.Now()
	g.AddGraphInputData(assessInd, "assessorInd")
	g.AddGraphInputData(ctinfos, "conntrack")
	g.AddGraphInputData(listen, "listen")
	g.AddGraphInputData(packet, "packet")
	g.AddGraphInputData(ratios, "ratio")
	responds, err := g.GetGraphOutputData("responds")
	if err != nil {
		t.Log(err)
	}
	t.Log(responds)
	endTime := time.Now()
	duration := endTime.Sub(startTime)
	t.Log(duration)
	//destroy g
	err = g.Destroy()
	if err != nil {
		fmt.Println(err)
	}
}

func TestDMS(t *testing.T) {
	graph_path := "./source/dms/dms_host_graph.yaml"
	//init g
	g, err := graph.InitialGraph(graph_path)
	if err != nil {
		t.Fatalf("failed to run g:%v", err)
	}
	//nicip
	nicip := map[string]string{"wlp1s0": "192.168.1.53"}
	nicSpeed := map[string]uint64{"wlp1s0": 1000}
	sidedataMap := make(map[string]interface{})
	sidedataMap["nicIP"] = nicip
	sidedataMap["nicSpeed"] = nicSpeed
	g.PrepareForRun(sidedataMap)
	g.Run()
	//assessInd
	devs := []string{"wlp1s0"}
	ag := dms.NewAssessorDataGenerator(devs)
	assessData, err := ag.GetAssessData()
	if err != nil {
		t.Log(err)
	}
	//ctinfos
	ctinfos, _ := dms.GetCtInfoConntrack()
	//listen ports
	listen := dms.GetListenInfos()
	//packet
	packet := map[string][]gopacket.Packet{
		"wlp1s0": make([]gopacket.Packet, 0),
	}
	ps, _ := dms.GetPackets("/home/yf/Workspace/pcap/hping3-icmp.pcap")
	packet["wlp1s0"] = ps
	//ratio
	ratios := map[string]float64{"wlp1s0": 0.01}
	//tcpconn
	conns := dms.GetTcpConnInfos()
	//add data
	startTime := time.Now()
	g.AddGraphInputData(assessData, "assessorData")
	g.AddGraphInputData(ctinfos, "conntrack")
	g.AddGraphInputData(listen, "listen")
	g.AddGraphInputData(packet, "packet")
	g.AddGraphInputData(ratios, "ratio")
	g.AddGraphInputData(conns, "tcpconn")
	responds, err := g.GetGraphOutputData("responds")
	if err != nil {
		t.Log(err)
	}
	t.Log(responds)
	endTime := time.Now()
	duration := endTime.Sub(startTime)
	t.Log(duration)
	//destroy g
	err = g.Destroy()
	if err != nil {
		fmt.Println(err)
	}
}
