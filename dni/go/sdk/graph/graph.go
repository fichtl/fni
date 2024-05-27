package graph

import (
	"fmt"
	"log"
	"sync"

	config "github.com/amianetworks/dni/sdk/config"
	flowmng "github.com/amianetworks/dni/sdk/flowmanager"
	"github.com/amianetworks/dni/sdk/node"
	"github.com/google/uuid"
)

type Graph struct {
	GraphID string // An ID automatically assigned at run time
	Status  string // The real-time state of Graph

	InputHandler         InputStreamHandler  // Handle all graph input streams
	OutputHandler        OutputStreamHandler // Handle all graph output streams
	InputSideDataHandler *GraphInputSideDataHandler
	OutputSideData       *flowmng.DataSlice

	SourceEdge          map[string][]int         // Source edge between graph input and nodes(key=source id, val=node id)
	NodeEdge            map[int]map[string][]int // Output edge between node and next nodes/graph output(key=pre node id, val=map[stream]nextNodes)
	SideDataSourceEdge  map[string][]int
	SideDataNodeEdge    map[int]map[string][]int
	StreamSortedNodes   []int
	SideDataSortedNodes []int
	Nodes               []*node.Node //Node in order
	wg                  sync.WaitGroup
}

func InitialGraph(graphfile string) (*Graph, error) {
	// parse yaml to get graph config
	gc, err := config.GetGraphConfig(graphfile)
	if err != nil {
		return nil, err
	}

	// parse graph config to get graph unit (parse stream "tag:index:name" to tagindex & name)
	gu, err := config.GetGraphUnit(gc)
	if err != nil {
		return nil, err
	}

	// get stream node edge between node and next nodes/graph output
	nodeEdge, streamSortedNodes, err := gu.GetNodeEdge()
	log.Printf("Node Edge:%v", nodeEdge)
	if err != nil {
		return nil, err
	}

	// get stream source edge between graph input and nodes
	sourceEdge, err := gu.GetSourceEdge()
	log.Printf("Source Edge:%v", sourceEdge)
	if err != nil {
		return nil, err
	}

	// get sidedata node edge between node and next nodes/graph output
	sideNodeEdge, sidedataSortedNodes, err := gu.GetSideNodeEdge()
	log.Printf("SideData Node Edge:%v", sideNodeEdge)
	if err != nil {
		return nil, err
	}

	// get sidedata source edge between graph input and nodes
	sideSourceEdge, err := gu.GetSideSourceEdge()
	log.Printf("SideData Source Edge:%v", sideSourceEdge)
	if err != nil {
		return nil, err
	}

	// init graph
	gr := &Graph{
		GraphID: uuid.New().String(),
		Status:  GRAPH_STATUS_NONE,

		SourceEdge:          sourceEdge,
		NodeEdge:            nodeEdge,
		SideDataSourceEdge:  sideSourceEdge,
		SideDataNodeEdge:    sideNodeEdge,
		StreamSortedNodes:   streamSortedNodes,
		SideDataSortedNodes: sidedataSortedNodes,
		Nodes:               make([]*node.Node, 0),
	}

	//init graph InputHandler
	gr.InputHandler = NewGraphInputHandler(gu.GraphInputStream)
	//init graph OutputHandler
	gr.OutputHandler = NewGraphOutputHandler(gu.GraphOutputStream)
	//init graph InputSideDataHandler
	gr.InputSideDataHandler = NewGraphInputSideDataHandler(gu.GraphInputSideData)
	//init graph Output SideData
	gr.OutputSideData = flowmng.NewDataSlice(gu.GraphOutputSideData)

	//init all nodes
	for nodeID, nodeUnit := range gu.Nodes {
		nodeSpec, err := node.InitNode(nodeID, nodeUnit)
		if err != nil {
			return nil, err
		}
		gr.Nodes = append(gr.Nodes, nodeSpec)
	}

	return gr, nil
}

func (g *Graph) LinkNodeEdge(preNodeID int, streamMap map[string][]int) {
	nextInputManagers := make(map[string][]*flowmng.InputManager)
	for stream, nextNodes := range streamMap {
		//to next node
		nextInputManagers[stream] = make([]*flowmng.InputManager, len(nextNodes))
		for id, nodeid := range nextNodes {
			nextInputManagers[stream][id] = g.Nodes[nodeid].GetInputManager()
		}
		//to graph output
		gOutMng := g.OutputHandler.GetInputManager()
		if gOutMng.HasInputStream(stream) {
			nextInputManagers[stream] = append(nextInputManagers[stream], gOutMng)
		}
	}
	g.Nodes[preNodeID].SetNodeEdge(nextInputManagers)
}

func (g *Graph) LinkSideDataNodeEdge(preNodeID int, sideDataMap map[string][]int) {
	nextInputSideData := make(map[string][]*flowmng.DataSlice)
	for sidedata, nextNodes := range sideDataMap {
		//to next node
		nextInputSideData[sidedata] = make([]*flowmng.DataSlice, len(nextNodes))
		for id, nodeid := range nextNodes {
			nextInputSideData[sidedata][id] = g.Nodes[nodeid].GetInputSideData()
		}
		//to graph output
		gOutSideData := g.OutputSideData
		if gOutSideData.HasName(sidedata) {
			nextInputSideData[sidedata] = append(nextInputSideData[sidedata], gOutSideData)
		}
	}
	g.Nodes[preNodeID].SetSideDataNodeEdge(nextInputSideData)
}

func (g *Graph) LinkSourceEdge() {
	gInStreams := g.InputHandler.GetGraphInputStreams()
	log.Printf("graph in strems:%v", gInStreams)
	nextInputManagers := make(map[string][]*flowmng.InputManager)
	for _, stream := range gInStreams {
		nextNodes := g.SourceEdge[stream]
		nextInputManagers[stream] = make([]*flowmng.InputManager, len(nextNodes))
		for id, nodeid := range nextNodes {
			nextInputManagers[stream][id] = g.Nodes[nodeid].InputManager
		}
	}
	g.InputHandler.SetSourceEdge(nextInputManagers)
}

func (g *Graph) LinkSideDataSourceEdge() {
	gInSideData := g.InputSideDataHandler.GetGraphInputSideData()
	log.Printf("graph input sidedata:%v", gInSideData)
	nextInputSideData := make(map[string][]*flowmng.DataSlice)
	for _, sidedata := range gInSideData {
		nextNodes := g.SideDataSourceEdge[sidedata]
		nextInputSideData[sidedata] = make([]*flowmng.DataSlice, len(nextNodes))
		for id, nodeid := range nextNodes {
			nextInputSideData[sidedata][id] = g.Nodes[nodeid].TaskContext.InputSideData
		}
	}
	g.InputSideDataHandler.SetSourceEdge(nextInputSideData)
}

func (g *Graph) PrepareForRun(gInputSideData map[string]interface{}) error {
	if err := g.InputHandler.PrepareForRun(); err != nil {
		return err
	}

	if err := g.OutputHandler.PrepareForRun(); err != nil {
		return err
	}

	//side data link
	g.LinkSideDataSourceEdge()
	for preNode, sideDataMap := range g.SideDataNodeEdge {
		g.LinkSideDataNodeEdge(preNode, sideDataMap)
	}

	//stream link
	g.LinkSourceEdge()
	for preNode, streamMap := range g.NodeEdge {
		g.LinkNodeEdge(preNode, streamMap)
	}

	//graph input sidedata
	gInputSideDataNames := g.InputSideDataHandler.GetGraphInputSideData()
	for _, sidedata := range gInputSideDataNames {
		data, ok := gInputSideData[sidedata]
		if !ok {
			return fmt.Errorf("sidedata (%s) not exist", sidedata)
		}
		err := g.AddGraphInputSideData(data, sidedata)
		if err != nil {
			return fmt.Errorf("sidedata (%s) transport error:%v", sidedata, err)
		}
	}

	// node prepare for run in topo sorted order
	for _, nid := range g.SideDataSortedNodes {
		if err := g.Nodes[nid].PrepareForRun(); err != nil {
			return fmt.Errorf("node %d prepare for run error:%v", nid, err)
		}
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
	if err := g.InputHandler.Close(); err != nil {
		log.Printf("close graph inputhandler failed:%v", err)
	}
	//stop all nodes executor and outputs in topo sorted order
	for _, nid := range g.StreamSortedNodes {
		if err := g.Nodes[nid].Destroy(); err != nil {
			log.Printf("destroy graph node %d failed:%v", nid, err)
		}
	}
	//close graph outputs
	if err := g.OutputHandler.Close(); err != nil {
		log.Printf("close graph inputhandler failed:%v", err)
	}

	//wait for all goroutine closed
	g.wg.Wait()
	log.Printf("destroy graph (%s)", g.GraphID)
	return nil
}

func (g *Graph) AddGraphInputData(data interface{}, stream string) error {
	d := flowmng.DataSpec{
		StreamName: stream,
		Data:       data,
	}
	err := g.InputHandler.AddGraphInputData(&d, stream)
	return err
}

func (g *Graph) AddGraphInputSideData(data interface{}, sidedata string) error {
	d := flowmng.DataSpec{
		StreamName: sidedata,
		Data:       data,
	}
	err := g.InputSideDataHandler.AddGraphInputSideData(&d, sidedata)
	return err
}

func (g *Graph) GetGraphOutputChannel(stream string) (chan *flowmng.DataSpec, bool) {
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

func (g *Graph) GetGraphOutputSideData(sidedata string) (interface{}, error) {
	data, ok := g.OutputSideData.GetByName(sidedata)
	if !ok {
		return nil, fmt.Errorf("gOutput sidedata(%s) not exist", sidedata)
	}
	return data.Data, nil
}
