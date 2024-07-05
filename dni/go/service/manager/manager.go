package manager

import (
	"fmt"
	"sync"

	"github.com/amianetworks/dni/sdk/config"
	"github.com/amianetworks/dni/sdk/graph"
)

type GraphManager struct {
	mutex    *sync.RWMutex
	GraphMap map[string]*graph.Graph
}

var gm GraphManager = GraphManager{
	mutex:    &sync.RWMutex{},
	GraphMap: make(map[string]*graph.Graph),
}

func RunGraph(name string, path string, sidedata map[string]interface{}) (string, error) {
	//check graph name
	gm.mutex.Lock()
	if _, ok := gm.GraphMap[name]; ok {
		return "", fmt.Errorf("graph (%s) already exist", name)
	}
	gm.mutex.Unlock()
	//init graph
	g, err := graph.InitialGraph(path)
	if err != nil {
		return "", err
	}
	if err := g.PrepareForRun(sidedata); err != nil {
		return "", err
	}
	if err := g.Run(); err != nil {
		return "", err
	}
	//add graph to graph map
	gm.mutex.Lock()
	gm.GraphMap[name] = g
	gm.mutex.Unlock()

	return name, nil
}

func GetGraphInputStreams(name string) config.StreamUnit {
	gm.mutex.Lock()
	g, ok := gm.GraphMap[name]
	gm.mutex.Unlock()
	if !ok {
		return config.StreamUnit{}
	}
	return g.GraphInputStream
}

func AddInputsToGraph(name string, inputs map[string]interface{}) error {
	gm.mutex.Lock()
	g := gm.GraphMap[name]
	gm.mutex.Unlock()
	g.AddAllGraphInputData(inputs)
	return nil
}

func GetGraphOutputs(name string) (map[string]interface{}, error) {
	gm.mutex.Lock()
	g := gm.GraphMap[name]
	gm.mutex.Unlock()
	outputs, err := g.GetAllGraphOutputData()
	if err != nil {
		return nil, err
	}
	return outputs, nil
}

func DestroyGraph(name string) error {
	gm.mutex.Lock()
	defer gm.mutex.Unlock()
	g, ok := gm.GraphMap[name]
	if !ok {
		return fmt.Errorf("graph (%s) does not exist", name)
	}
	err := g.Destroy()
	if err != nil {
		return err
	}
	delete(gm.GraphMap, name)
	return nil
}
