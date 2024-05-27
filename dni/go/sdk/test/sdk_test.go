package test

import (
	"fmt"
	"testing"

	graph "github.com/amianetworks/dni/sdk/graph"
	ort "github.com/yalue/onnxruntime_go"
)

func TestGraphAPI1(t *testing.T) {
	graph_path := "01-g.yaml"
	//init g
	g, err := graph.InitialGraph(graph_path)
	if err != nil {
		t.Fatalf("failed to run g:%s", g.GraphID)
	}
	g.PrepareForRun(make(map[string]interface{}))
	g.Run()
	//add inputs
	for i := 0; i < 20000; i++ {
		g.AddGraphInputData(float64(1.0), "A")
		g.AddGraphInputData(float64(2.0), "B")
		//get output
		d, err := g.GetGraphOutputData("C")
		if err != nil {
			t.Log(err)
		}
		fmt.Println("C:", d)
	}
	//destroy g
	err = g.Destroy()
	if err != nil {
		fmt.Println(err)
	}
}

func TestGraphAPI2(t *testing.T) {
	//init、run、destory g
	graph_path := "02-g.yaml"
	g, err := graph.InitialGraph(graph_path)
	if err != nil {
		t.Fatalf("failed to run g:%s", g.GraphID)
	}
	g.PrepareForRun(make(map[string]interface{}))
	g.AddGraphInputData(float64(1), "A")
	g.AddGraphInputData(float64(2), "B")
	g.AddGraphInputData(float64(3), "B")
	g.Run()
	d, err := g.GetGraphOutputData("C")
	if err != nil {
		t.Log(err)
	}
	fmt.Println("C:", d)
	d, err = g.GetGraphOutputData("D")
	if err != nil {
		t.Log(err)
	}
	fmt.Println("D:", d)
	err = g.Destroy()
	if err != nil {
		fmt.Println(err)
	}
}

func TestGraphAPI3(t *testing.T) {
	graph_path := "03-g.yaml"
	g, err := graph.InitialGraph(graph_path)
	if err != nil {
		t.Fatalf("failed to run g:%s", g.GraphID)
	}
	g.PrepareForRun(make(map[string]interface{}))
	g.AddGraphInputData(2, "A")
	g.AddGraphInputData(2, "B")
	g.Run()
	d, err := g.GetGraphOutputData("C")
	if err != nil {
		t.Log(err)
	}
	fmt.Println("C:", d)
	d, err = g.GetGraphOutputData("D")
	if err != nil {
		t.Log(err)
	}
	fmt.Println("D:", d)
	d, err = g.GetGraphOutputData("E")
	if err != nil {
		t.Log(err)
	}
	fmt.Println("E:", d)
	err = g.Destroy()
	if err != nil {
		fmt.Println(err)
	}
}

func TestGraphAPI4(t *testing.T) {
	//init onnx enviroment
	var onnxruntimeLibPath = "/home/yf/workspace/dni/dni/go/third_party/onnxruntime.so"
	ort.SetSharedLibraryPath(onnxruntimeLibPath)
	err := ort.InitializeEnvironment()
	if err != nil {
		t.Errorf("initialize the onnxruntime library error: %v", err)
		t.Fail()
	}
	//init、run、destroy g
	graph_path := "04-g.yaml"
	g, err := graph.InitialGraph(graph_path)
	if err != nil {
		t.Fatalf("failed to run g:%s", g.GraphID)
	}
	g.PrepareForRun(make(map[string]interface{}))
	a := [][]float32{{1, 2, 3, 4}, {3, 3, 3, 3}, {2, 2, 2, 2}}
	g.AddGraphInputData(a, "A")
	g.Run()
	d, err := g.GetGraphOutputData("B")
	if err != nil {
		t.Log(err)
	}
	outputs := d.([]ort.ArbitraryTensor)
	for i := 0; i < len(outputs); i++ {
		outdata := outputs[i].(*ort.Tensor[float32]).GetData()
		fmt.Printf("B out %d:%v\n", i, outdata)
	}
	err = g.Destroy()
	if err != nil {
		fmt.Println(err)
	}
}

func TestGraphAPI5(t *testing.T) {
	//init、run、destroy g
	graph_path := "05-g.yaml"
	g, err := graph.InitialGraph(graph_path)
	if err != nil {
		t.Fatalf("failed to run g:%s", g.GraphID)
	}
	g.PrepareForRun(make(map[string]interface{}))
	g.AddGraphInputData(1, "A")
	g.AddGraphInputData(2, "B")
	g.Run()
	d, err := g.GetGraphOutputData("C")
	if err != nil {
		t.Log(err)
	}
	fmt.Println("C:", d)
	d, err = g.GetGraphOutputData("D")
	if err != nil {
		t.Log(err)
	}
	fmt.Println("D:", d)
	d, err = g.GetGraphOutputData("E")
	if err != nil {
		t.Log(err)
	}
	fmt.Println("E:", d)
	d, err = g.GetGraphOutputData("F")
	if err != nil {
		t.Log(err)
	}
	fmt.Println("F:", d)
	d, err = g.GetGraphOutputData("G")
	if err != nil {
		t.Log(err)
	}
	fmt.Println("G:", d)
	err = g.Destroy()
	if err != nil {
		fmt.Println(err)
	}
}

func TestDestroyAPI1(t *testing.T) {
	//init and destroy g
	graph_path := "01-g.yaml"
	//初始化
	g, err := graph.InitialGraph(graph_path)
	if err != nil {
		t.Fatalf("failed to run g:%s", g.GraphID)
	}
	//准备
	g.PrepareForRun(make(map[string]interface{}))
	//销毁图
	err = g.Destroy()
	if err != nil {
		fmt.Println(err)
	}
}

func TestDestroyAPI2(t *testing.T) {
	//init and destroy g
	graph_path := "01-g.yaml"
	g, err := graph.InitialGraph(graph_path)
	if err != nil {
		t.Fatalf("failed to run g:%s", g.GraphID)
	}
	g.PrepareForRun(make(map[string]interface{}))
	g.AddGraphInputData(1, "A")
	g.AddGraphInputData(2, "B")
	err = g.Destroy()
	if err != nil {
		fmt.Println(err)
	}
}

func TestSideData1(t *testing.T) {
	graph_path := "01-sidedata.yaml"
	g, err := graph.InitialGraph(graph_path)
	if err != nil {
		t.Fatalf("failed to run graph:%v", err)
		return
	}
	sidedataMap := make(map[string]interface{})
	sidedataMap["in_sidedata"] = int(10)
	err = g.PrepareForRun(sidedataMap)
	if err != nil {
		t.Fatalf("failed to prepare graph:%v", err)
	}
	g.Run()
	g.AddGraphInputData(1, "A")
	d, err := g.GetGraphOutputData("B")
	if err != nil {
		t.Fatalf("failed to get graph output")
	}
	fmt.Println("output data:", d)
}

func TestSideData2(t *testing.T) {
	graph_path := "02-sidedata.yaml"
	g, err := graph.InitialGraph(graph_path)
	if err != nil {
		t.Fatalf("failed to run graph:%v", err)
		return
	}
	sidedataMap := make(map[string]interface{})
	sidedataMap["in_sidedata"] = int(10)
	err = g.PrepareForRun(sidedataMap)
	if err != nil {
		t.Fatalf("failed to prepare graph:%v", err)
	}
	g.Run()
	g.AddGraphInputData(1, "A")
	d, err := g.GetGraphOutputData("D")
	if err != nil {
		t.Fatalf("failed to get graph output")
	}
	fmt.Println("output data:", d)
}
