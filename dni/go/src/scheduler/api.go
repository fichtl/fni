package scheduler

import (
	"fmt"
)

func RunGraph(graphfile string) (string, error) {
	graph, err := InitialGraph(graphfile)
	if err != nil {
		return "", err
	}
	if err := graph.GetReady(); err != nil {
		return "", err
	}
	if err := graph.Run(); err != nil {
		return "", err
	}
	return graph.GraphID, nil
}

func PauseGraph(id string) error {
	gm.Mutex.Lock()
	defer gm.Mutex.Unlock()
	gr, ok := gm.Graphs[id]
	if !ok {
		return fmt.Errorf("graph (%s) does not exist", id)
	}
	return gr.Pause()
}

func RerunGraph(id string) error {
	gm.Mutex.Lock()
	defer gm.Mutex.Unlock()

	gr, ok := gm.Graphs[id]
	if !ok {
		return fmt.Errorf("graph (%s) does not exist", id)
	}
	return gr.Run()
}

func DestroyGraph(id string) error {
	gm.Mutex.Lock()
	gr, ok := gm.Graphs[id]
	gm.Mutex.Unlock()
	if !ok {
		return fmt.Errorf("graph (%s) does not exist", id)
	}
	return gr.Destroy()
}
