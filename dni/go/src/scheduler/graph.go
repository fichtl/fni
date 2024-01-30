package scheduler

import (
	"fmt"
	"strings"
	"sync"

	"github.com/amianetworks/am.modules/log"
	flowmng "github.com/amianetworks/dni/src/flowmanager"
	"github.com/amianetworks/dni/src/graph"
	"github.com/amianetworks/dni/src/node"
	"github.com/google/uuid"
)

// 图的结构
type Graph struct {
	GraphID string // An ID automatically assigned at run time
	Status  string // The real-time state of Graph

	InputHandler  []InputStreamHandler  // 用于管理整张图的输入流，handler根据图输入生成输入流，输入流会往下流入每个节点进行计算
	OutputHandler []OutputStreamHandler // 用于管理整张图的输出流，将节点最终的处理结果进行输出处理

	SourceEdge map[int][]int // 用于构建图输入流与节点的边，图的输入流流入到指定节点，key=source id, val=node id
	NodesEdge  map[int][]int // 用于构建节点之间的边，父节点的输出流向子结点流动，key=parent node id, val=son node id
	Nodes      []*node.Node  // 每个节点的具体信息，节点按顺序排列

	// close graph
	mutex *sync.Mutex   // 用于锁住refs的值
	refs  int           // 当前处于执行状态的节点个数
	stop  chan struct{} // 停止运行图的flag
}

// 管理DNI服务的所有图
type GraphManager struct {
	Mutex  *sync.Mutex
	Graphs map[string]*Graph // key=GraphID
}

var gm = GraphManager{
	Mutex:  &sync.Mutex{},
	Graphs: make(map[string]*Graph),
}

// 根据graph文件初始化图结构
func InitialGraph(graphfile string) (*Graph, error) {
	// 解析yaml文件，获取图配置
	gc, err := graph.GetGraphConfig(graphfile)
	if err != nil {
		return nil, err
	}
	// 获取图节点的边信息
	nodeEdge, err := gc.GetNodeEdge()
	if err != nil {
		return nil, err
	}
	// 获取图输入与图节点的边信息
	sourceEdge, err := gc.GetSourceEdge()
	if err != nil {
		return nil, err
	}
	// 初始化图
	gr := &Graph{
		GraphID: uuid.New().String(),
		Status:  GRAPH_STATUS_NONE,

		SourceEdge: sourceEdge,
		NodesEdge:  nodeEdge,
		Nodes:      make([]*node.Node, 0),
		mutex:      &sync.Mutex{},
	}
	// 初始化inputHandler和outputHandler
	for _, graphIn := range gc.GraphInputStream {
		var inputHandler InputStreamHandler
		switch strings.Split(graphIn.Name, ":")[0] {
		case graph.GRAPH_PCAPFILE_INPUT:
			inputHandler = InitPCAPFileHandler(graphIn)
		case graph.GRAPH_DB_INPUT:
			inputHandler = InitDBHandler(graphIn)
		}
		gr.InputHandler = append(gr.InputHandler, inputHandler)
	}

	for _, graphout := range gc.GraphOutputStream {
		var outputHandler OutputStreamHandler
		switch graphout.Name {
		case graph.GRAPH_DB_OUTPUT:
			outputHandler = InitGraphDBOutput(graphout)
		case graph.GRAPH_FILE_OUTPUT:
			outputHandler = InitGraphFileOutput(graphout)
		}
		gr.OutputHandler = append(gr.OutputHandler, outputHandler)
	}
	// 初始化每个node
	for nodeID, nodeConf := range gc.Nodes {
		nodeSpec, err := node.InitNode(nodeID, nodeConf)
		if err != nil {
			return nil, err
		}
		gr.Nodes = append(gr.Nodes, nodeSpec)
	}

	gm.Mutex.Lock()
	gm.Graphs[gr.GraphID] = gr
	gm.Mutex.Unlock()
	return gr, nil
}

// 连接每个节点之间的边，即搭建父节点和子节点之间的数据通道
func (g *Graph) LinkNodeEdge(parentNodeID, sonNodeID int) error {
	outMng := g.Nodes[parentNodeID].OutputManager
	inMng := g.Nodes[sonNodeID].InputManager

	go func() {
		for d := range outMng.Output {
			inMng.AddPacket(d)
		}
		log.R.Infof("graph <%s> data channel between (node%d) and (node%d) has been closed\n", g.GraphID, parentNodeID, sonNodeID)
	}()

	return nil
}

// 连接图输入流与节点之间的边
func (g *Graph) LinkSourceEdge() error {
	for id, input := range g.InputHandler {

		toNodes, ok := g.SourceEdge[id]
		if !ok {
			return fmt.Errorf("something wrong with source edge")
		}

		inMngs := make([]*flowmng.InputManager, 0)
		for _, inNodeID := range toNodes {
			inputMng := g.Nodes[inNodeID].InputManager
			inMngs = append(inMngs, inputMng)
		}

		dataChan := input.GetDataChannel()
		go func() {
			for srcdata := range dataChan {
				for _, mng := range inMngs {
					mng.AddPacket(srcdata)
				}
			}
			log.R.Info("delete edge between source and node")
		}()
	}
	return nil
}

// 处理最后一个节点的输出
func (g *Graph) LinkOutputSink() error {
	lastNode := g.Nodes[len(g.Nodes)-1]

	go func() {
		log.R.Info("construct output sink")
		for data := range lastNode.OutputManager.Output {
			for _, outhandle := range g.OutputHandler {
				if err := outhandle.SendToGraphOutput(data); err != nil {
					log.R.Errorf("send data to graph output failed, err=%v", err)
					return
				}
			}
		}
		log.R.Info("close output sink")
	}()

	return nil
}

// 图初始化完成后，在运行前，需要完成准备工作
func (g *Graph) GetReady() error {
	for _, inHandle := range g.InputHandler {
		if err := inHandle.PrepareForRun(); err != nil {
			return err
		}
	}
	for _, outHandle := range g.OutputHandler {
		if err := outHandle.PrepareForRun(); err != nil {
			return err
		}
	}
	if err := g.LinkSourceEdge(); err != nil {
		return err
	}
	if err := g.LinkOutputSink(); err != nil {
		return err
	}
	for inNodeID, outNodes := range g.NodesEdge {
		for _, outNodeID := range outNodes {
			if err := g.LinkNodeEdge(inNodeID, outNodeID); err != nil {
				return err
			}
		}
	}
	g.setStatusReady()
	return nil
}

func (g *Graph) Run() error {

	// TODO：判断当前graph的状态
	if g.Status != GRAPH_STATUS_READY {
		return fmt.Errorf("GRAPH is not ready")
	}

	// 运行前先把停止器初始化好
	g.stop = make(chan struct{})

	// 由后往前 运行节点任务
	g.refs = len(g.Nodes)
	log.R.Debugf("total number of node task: %d", g.refs)
	g.setStatusRunning()
	for id := g.refs - 1; id >= 0; id-- {
		n := g.Nodes[id]
		go func() {
			err := n.Execute()
			if err != nil {
				g.setStatusFailed()
				log.R.Errorf("execute <%s> task failed, err: %v", n.NodeID, err)
			}
			g.mutex.Lock()
			g.refs--
			if g.refs == 0 {
				close(g.stop)
				log.R.Debug("close graph stop channel")
			}
			g.mutex.Unlock()
			log.R.Infof("graph (%s) node%d task exit", g.GraphID, n.NodeID)
		}()
	}

	for _, inHandle := range g.InputHandler {
		if err := inHandle.GenerateInputStream(); err != nil {
			g.setStatusFailed()
			return err
		}
	}
	return nil
}

func (g *Graph) Pause() error {
	if g.refs != 0 {
		for _, n := range g.Nodes {
			n.Stop()
		}
	}
	<-g.stop
	log.R.Infof("graph (%s) task pause", g.GraphID)
	return nil
}

func (g *Graph) Destroy() error {
	g.Pause()

	for _, inHandle := range g.InputHandler {
		inHandle.Close()
	}
	for _, n := range g.Nodes {
		n.Destroy()
	}
	for _, outhandle := range g.OutputHandler {
		outhandle.Close()
	}

	gm.Mutex.Lock()
	delete(gm.Graphs, g.GraphID)
	gm.Mutex.Unlock()

	log.R.Infof("destroy graph (%s)", g.GraphID)
	return nil
}
