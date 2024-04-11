package test

import (
	"fmt"
	"testing"

	"github.com/amianetworks/dni/sdk/runner"
	"github.com/amianetworks/dni/sdk/scheduler"
	ort "github.com/yalue/onnxruntime_go"
)

func TestGraphAPI1(t *testing.T) {
	graph_path := "01-graph.yaml"
	//register executors
	runner.InitExecutorRegistry()
	//init graph
	graph, err := scheduler.InitialGraph(graph_path)
	if err != nil {
		t.Fatalf("failed to run graph:%s", graph.GraphID)
	}
	graph.GetReady()
	graph.Run()
	//add inputs
	for i := 0; i < 20000; i++ {
		graph.AddGraphInputData(float64(1.0), "A")
		graph.AddGraphInputData(float64(2.0), "B")
		//get output
		d, err := graph.GetGraphOutputData("C")
		if err != nil {
			t.Log(err)
		}
		fmt.Println("C:", d)
	}
	//destroy graph
	err = graph.Destroy()
	if err != nil {
		fmt.Println(err)
	}
}

func TestGraphAPI2(t *testing.T) {
	//register executors
	runner.InitExecutorRegistry()
	//init、run、destory graph
	graph_path := "02-graph.yaml"
	graph, err := scheduler.InitialGraph(graph_path)
	if err != nil {
		t.Fatalf("failed to run graph:%s", graph.GraphID)
	}
	graph.GetReady()
	graph.AddGraphInputData(float64(1), "A")
	graph.AddGraphInputData(float64(2), "B")
	graph.AddGraphInputData(float64(3), "B")
	graph.Run()
	d, err := graph.GetGraphOutputData("C")
	if err != nil {
		t.Log(err)
	}
	fmt.Println("C:", d)
	d, err = graph.GetGraphOutputData("D")
	if err != nil {
		t.Log(err)
	}
	fmt.Println("D:", d)
	err = graph.Destroy()
	if err != nil {
		fmt.Println(err)
	}
}

func TestGraphAPI3(t *testing.T) {
	//register executors
	runner.InitExecutorRegistry()

	graph_path := "03-graph.yaml"
	graph, err := scheduler.InitialGraph(graph_path)
	if err != nil {
		t.Fatalf("failed to run graph:%s", graph.GraphID)
	}
	graph.GetReady()
	graph.AddGraphInputData(2, "A")
	graph.AddGraphInputData(2, "B")
	graph.Run()
	d, err := graph.GetGraphOutputData("C")
	if err != nil {
		t.Log(err)
	}
	fmt.Println("C:", d)
	d, err = graph.GetGraphOutputData("D")
	if err != nil {
		t.Log(err)
	}
	fmt.Println("D:", d)
	d, err = graph.GetGraphOutputData("E")
	if err != nil {
		t.Log(err)
	}
	fmt.Println("E:", d)
	err = graph.Destroy()
	if err != nil {
		fmt.Println(err)
	}
}

func TestGraphAPI4(t *testing.T) {
	//register executors
	runner.InitExecutorRegistry()
	//init onnx enviroment
	var onnxruntimeLibPath = "/home/yf/workspace/dni/dni/go/third_party/onnxruntime.so"
	ort.SetSharedLibraryPath(onnxruntimeLibPath)
	err := ort.InitializeEnvironment()
	if err != nil {
		t.Errorf("initialize the onnxruntime library error: %v", err)
		t.Fail()
	}
	//init、run、destroy graph
	graph_path := "04-graph.yaml"
	graph, err := scheduler.InitialGraph(graph_path)
	if err != nil {
		t.Fatalf("failed to run graph:%s", graph.GraphID)
	}
	graph.GetReady()
	a := [][]float32{{1, 2, 3, 4}, {3, 3, 3, 3}, {2, 2, 2, 2}}
	graph.AddGraphInputData(a, "A")
	graph.Run()
	d, err := graph.GetGraphOutputData("B")
	if err != nil {
		t.Log(err)
	}
	outputs := d.([]ort.ArbitraryTensor)
	for i := 0; i < len(outputs); i++ {
		outdata := outputs[i].(*ort.Tensor[float32]).GetData()
		fmt.Printf("B out %d:%v\n", i, outdata)
	}
	err = graph.Destroy()
	if err != nil {
		fmt.Println(err)
	}
}

func TestGraphAPI5(t *testing.T) {
	//register executors
	runner.InitExecutorRegistry()
	//init、run、destroy graph
	graph_path := "05-graph.yaml"
	graph, err := scheduler.InitialGraph(graph_path)
	if err != nil {
		t.Fatalf("failed to run graph:%s", graph.GraphID)
	}
	graph.GetReady()
	graph.AddGraphInputData(1, "A")
	graph.AddGraphInputData(2, "B")
	graph.Run()
	d, err := graph.GetGraphOutputData("C")
	if err != nil {
		t.Log(err)
	}
	fmt.Println("C:", d)
	d, err = graph.GetGraphOutputData("D")
	if err != nil {
		t.Log(err)
	}
	fmt.Println("D:", d)
	d, err = graph.GetGraphOutputData("E")
	if err != nil {
		t.Log(err)
	}
	fmt.Println("E:", d)
	d, err = graph.GetGraphOutputData("F")
	if err != nil {
		t.Log(err)
	}
	fmt.Println("F:", d)
	d, err = graph.GetGraphOutputData("G")
	if err != nil {
		t.Log(err)
	}
	fmt.Println("G:", d)
	err = graph.Destroy()
	if err != nil {
		fmt.Println(err)
	}
}

func TestDestroyAPI1(t *testing.T) {
	//register executors
	runner.InitExecutorRegistry()
	//init and destroy graph
	graph_path := "01-graph.yaml"
	//初始化
	graph, err := scheduler.InitialGraph(graph_path)
	if err != nil {
		t.Fatalf("failed to run graph:%s", graph.GraphID)
	}
	//准备
	graph.GetReady()
	//销毁图
	err = graph.Destroy()
	if err != nil {
		fmt.Println(err)
	}
}

func TestDestroyAPI2(t *testing.T) {
	//register executors
	runner.InitExecutorRegistry()
	//init and destroy graph
	graph_path := "01-graph.yaml"
	graph, err := scheduler.InitialGraph(graph_path)
	if err != nil {
		t.Fatalf("failed to run graph:%s", graph.GraphID)
	}
	graph.GetReady()
	graph.AddGraphInputData(1, "A")
	graph.AddGraphInputData(2, "B")
	err = graph.Destroy()
	if err != nil {
		fmt.Println(err)
	}
}
