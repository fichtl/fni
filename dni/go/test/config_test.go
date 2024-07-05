package test

import (
	"testing"

	config "github.com/amianetworks/dni/sdk/config"
)

func TestParseGraph(t *testing.T) {
	graphfile := "/home/yf/workspace/dni/dni/go/sdk/test/pcap_parse.yaml"
	gc, err := config.GetGraphConfig(graphfile)
	if err != nil {
		t.Fatal(err)
	}
	t.Log(gc.Nodes)
	gu, err := config.GetGraphUnit(gc)
	if err != nil {
		t.Fatal(err)
	}
	t.Log(gu.Nodes)
}

func TestGetSoucreEdge1(t *testing.T) {
	graphfile := "/home/yf/workspace/dni/dni/go/sdk/test/pcap_parse.yaml"
	gc, err := config.GetGraphConfig(graphfile)
	if err != nil {
		t.Fatal(err)
	}
	gu, err := config.GetGraphUnit(gc)
	if err != nil {
		t.Fatal(err)
	}
	sourceEdge, err := gu.GetSourceEdge()
	if err != nil {
		t.Fatal(err)
	}
	t.Log(sourceEdge)
}

func TestGetSoucreEdge2(t *testing.T) {
	graphfile := "/home/yf/workspace/dni/dni/go/sdk/test/sip_base_merge.yaml"
	gc, err := config.GetGraphConfig(graphfile)
	if err != nil {
		t.Fatal(err)
	}
	gu, err := config.GetGraphUnit(gc)
	if err != nil {
		t.Fatal(err)
	}
	sourceEdge, err := gu.GetSourceEdge()
	if err != nil {
		t.Fatal(err)
	}
	t.Log(sourceEdge)
}

func TestGetSoucreEdge3(t *testing.T) {
	graphfile := "/home/yf/workspace/dni/dni/go/sdk/test/ip_merge.yaml"
	gc, err := config.GetGraphConfig(graphfile)
	if err != nil {
		t.Fatal(err)
	}
	gu, err := config.GetGraphUnit(gc)
	if err != nil {
		t.Fatal(err)
	}
	sourceEdge, err := gu.GetSourceEdge()
	if err != nil {
		t.Fatal(err)
	}
	t.Log(sourceEdge)
}

func TestGetNodeEdge1(t *testing.T) {
	graphfile := "/home/yf/workspace/dni/dni/go/sdk/test/pcap_parse.yaml"
	gc, err := config.GetGraphConfig(graphfile)
	if err != nil {
		t.Fatal(err)
	}
	gu, err := config.GetGraphUnit(gc)
	if err != nil {
		t.Fatal(err)
	}
	nodeEdge, topo, err := gu.GetNodeEdge()
	if err != nil {
		t.Fatal(err)
	}
	t.Log(nodeEdge)
	t.Log(topo)
}

func TestGetNodeEdge2(t *testing.T) {
	graphfile := "/home/yf/workspace/dni/dni/go/sdk/test/sip_base_merge.yaml"
	gc, err := config.GetGraphConfig(graphfile)
	if err != nil {
		t.Fatal(err)
	}
	gu, err := config.GetGraphUnit(gc)
	if err != nil {
		t.Fatal(err)
	}
	nodeEdge, topo, err := gu.GetNodeEdge()
	if err != nil {
		t.Fatal(err)
	}
	t.Log(nodeEdge)
	t.Log(topo)
}

func TestGetNodeEdge3(t *testing.T) {
	graphfile := "/home/yf/workspace/dni/dni/go/sdk/test/ip_merge.yaml"
	gc, err := config.GetGraphConfig(graphfile)
	if err != nil {
		t.Fatal(err)
	}
	gu, err := config.GetGraphUnit(gc)
	if err != nil {
		t.Fatal(err)
	}
	nodeEdge, topo, err := gu.GetNodeEdge()
	if err != nil {
		t.Fatal(err)
	}
	t.Log(nodeEdge)
	t.Log(topo)
}

func TestGetNodeEdge4(t *testing.T) {
	graphfile := "/home/yf/workspace/dni/dni/go/sdk/test/01-loop-graph.yaml"
	gc, err := config.GetGraphConfig(graphfile)
	if err != nil {
		t.Fatal(err)
	}
	gu, err := config.GetGraphUnit(gc)
	if err != nil {
		t.Fatal(err)
	}
	nodeEdge, topo, err := gu.GetNodeEdge()
	if err != nil {
		t.Fatal(err)
	}
	t.Log(nodeEdge)
	t.Log(topo)
}

func TestGetNodeEdge5(t *testing.T) {
	graphfile := "/home/yf/workspace/dni/dni/go/sdk/test/01-loop-graph.yaml"
	gc, err := config.GetGraphConfig(graphfile)
	if err != nil {
		t.Fatal(err)
	}
	gu, err := config.GetGraphUnit(gc)
	if err != nil {
		t.Fatal(err)
	}
	nodeEdge, topo, err := gu.GetNodeEdge()
	if err != nil {
		t.Fatal(err)
	}
	t.Log(nodeEdge)
	t.Log(topo)
}
