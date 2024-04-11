package scheduler

import (
	"fmt"
	"log"
	"sync"
	"time"

	flowmng "github.com/amianetworks/dni/sdk/flowmanager"
	"github.com/amianetworks/dni/sdk/graph"
	"github.com/amianetworks/dni/sdk/node"
	"github.com/google/uuid"
)

type Graph struct {
	GraphID string // An ID automatically assigned at run time
	Status  string // The real-time state of Graph

	InputHandler  InputStreamHandler  // Handle all graph input streams
	OutputHandler OutputStreamHandler // Handle all graph output streams

	SourceEdge map[int][]int            // Source edge between graph input and nodes(key=source id, val=node id)
	OutputEdge map[int]map[string][]int // Output edge between node and next nodes/graph output(key=pre node id, val=map[stream]nextNodes)

	Nodes []*node.Node //Node in order
	wg    sync.WaitGroup
}

func InitialGraph(graphfile string) (*Graph, error) {
	// parse yaml to get graph config
	gc, err := graph.GetGraphConfig(graphfile)
	if err != nil {
		return nil, err
	}
	// get output edge between node and next nodes/graph output
	outputEdge, err := gc.GetOutputEdge()
	if err != nil {
		return nil, err
	}
	// get source edge between graph input and nodes
	sourceEdge, err := gc.GetSourceEdge()
	if err != nil {
		return nil, err
	}
	// init graph
	gr := &Graph{
		GraphID: uuid.New().String(),
		Status:  GRAPH_STATUS_NONE,

		SourceEdge: sourceEdge,
		OutputEdge: outputEdge,
		Nodes:      make([]*node.Node, 0),
	}
	//init graph InputHandler
	gInputStreams := make([]string, 0)
	for _, gInUnit := range gc.GraphInputStream {
		gInputStreams = append(gInputStreams, gInUnit.Name)
	}
	gr.InputHandler = NewGraphInputHandler(gInputStreams)

	//init graph OutputHandler
	gOutputStreams := make([]string, 0)
	for _, gOutUnit := range gc.GraphOutputStream {
		gOutputStreams = append(gOutputStreams, gOutUnit.Name)
	}
	gr.OutputHandler = NewGraphOutputHandler(gOutputStreams)
	//init all nodes
	for nodeID, nodeConf := range gc.Nodes {
		nodeSpec, err := node.InitNode(nodeID, nodeConf)
		if err != nil {
			return nil, err
		}
		gr.Nodes = append(gr.Nodes, nodeSpec)
	}

	return gr, nil
}

func (g *Graph) LinkOutputEdge(preNodeID int, streamMap map[string][]int) {
	nextInputManagers := make(map[string][]*flowmng.InputManager)
	for stream, nextNodes := range streamMap {
		//to next node
		nextInputManagers[stream] = make([]*flowmng.InputManager, len(nextNodes))
		for id, nodeid := range nextNodes {
			nextInputManagers[stream][id] = g.Nodes[nodeid].InputManager
		}
		//to graph output
		gOutMng := g.OutputHandler.GetInputManager()
		if gOutMng.HasInputStream(stream) {
			nextInputManagers[stream] = append(nextInputManagers[stream], gOutMng)
		}
	}
	g.Nodes[preNodeID].OutputManager.NextInputManagers = nextInputManagers
}

func (g *Graph) LinkSourceEdge() {
	gInStreams := g.InputHandler.GetGraphInputStreams()
	nextInputManagers := make(map[string][]*flowmng.InputManager)
	for i, stream := range gInStreams {
		nextNodes := g.SourceEdge[i]
		nextInputManagers[stream] = make([]*flowmng.InputManager, len(nextNodes))
		for id, nodeid := range nextNodes {
			nextInputManagers[stream][id] = g.Nodes[nodeid].InputManager
		}
	}
	g.InputHandler.SetSourceEdge(nextInputManagers)
}

func (g *Graph) GetReady() error {
	if err := g.InputHandler.PrepareForRun(); err != nil {
		return err
	}

	if err := g.OutputHandler.PrepareForRun(); err != nil {
		return err
	}

	g.LinkSourceEdge()

	for preNode, streamMap := range g.OutputEdge {
		g.LinkOutputEdge(preNode, streamMap)
	}

	g.setStatusReady()
	return nil
}

func (g *Graph) Run() error {
	// TODO: judge g status
	if g.Status != GRAPH_STATUS_READY {
		return fmt.Errorf("GRAPH is not ready")
	}

	g.setStatusRunning()
	for id := len(g.Nodes) - 1; id >= 0; id-- {
		n := g.Nodes[id]
		g.wg.Add(1)
		go func() {
			err := n.Execute()
			if err != nil {
				g.setStatusFailed()
				log.Printf("execute <%d> task failed, err: %v", n.NodeID, err)
			}
			log.Printf("graph (%s) node%d task exit", g.GraphID, n.NodeID)
			g.wg.Done()
		}()
	}

	return nil
}

func (g *Graph) Destroy() error {
	//close graph input channel
	_ = g.InputHandler.Close()
	//stop all nodes executor and outputs
	for _, n := range g.Nodes {
		_ = n.Destroy()
	}
	//close graph outputs
	_ = g.OutputHandler.Close()

	//wait for all goroutine closed
	g.wg.Wait()
	log.Printf("destroy graph (%s)", g.GraphID)
	return nil
}

func (g *Graph) GetGraphOutputChannel(stream string) (chan flowmng.DataSpec, bool) {
	gOutCh, ok := g.OutputHandler.GetOutputChannel(stream)
	return gOutCh, ok
}

func (g *Graph) GetGraphOutputData(stream string) (interface{}, error) {
	gOutCh, ok := g.OutputHandler.GetOutputChannel(stream)
	if !ok {
		return nil, fmt.Errorf("gOutput stream:%s not exist", stream)
	}
	data, ok := <-gOutCh
	if !ok {
		return nil, fmt.Errorf("gOutput stream:%s channel is closed", stream)
	}
	return data.Data, nil
}

func (g *Graph) AddGraphInputData(data interface{}, stream string) error {
	d := flowmng.DataSpec{
		StreamName: stream,
		TimeStamp:  time.Now(),
		Data:       data,
	}
	err := g.InputHandler.AddGraphInputData(d, stream)
	return err
}
