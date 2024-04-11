package graph

import (
	"testing"
)

func TestParseGraph(t *testing.T) {
	graphfile := "./graph.yaml"
	gc, err := GetGraphConfig(graphfile)
	if err != nil {
		t.Fatal(err)
	}
	t.Log(*gc)
}

func TestParseGraph2(t *testing.T) {
	graphfile := "./graph2.yaml"
	gc, err := GetGraphConfig(graphfile)
	if err != nil {
		t.Fatal(err)
	}
	t.Log(*gc)
}

func TestParseGraph3(t *testing.T) {
	graphfile := "./graph3.yaml"
	gc, err := GetGraphConfig(graphfile)
	if err != nil {
		t.Fatal(err)
	}
	t.Log(*gc)
}

func TestReadGraphConfig(t *testing.T) {
	graphfile := "02-graph.yaml"
	gc, err := GetGraphConfig(graphfile)
	if err != nil {
		t.Fatal(err)
	}
	t.Log(gc)

	output, err := gc.GetOutputEdge()
	if err != nil {
		t.Fatal(err)
	}
	t.Log(output)

	src, err := gc.GetSourceEdge()
	if err != nil {
		t.Fatal(err)
	}
	t.Log(src)

}
