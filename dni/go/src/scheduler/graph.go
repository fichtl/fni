package scheduler

import (
	"fmt"
	"sync"

	"github.com/amianetworks/am.modules/log"
	"github.com/amianetworks/dni/src/data"
	"github.com/amianetworks/dni/src/design"
	"github.com/amianetworks/dni/src/graph"
	"github.com/amianetworks/dni/src/node"
	"github.com/google/uuid"
)

// 图的状态
const (
	GRAPH_STATUS_NONE        string = "none"        // 图未完成初始化，空图
	GRAPH_STATUS_INITIALIZED string = "initialized" // 图已完成初始化
	GRAPH_STATUS_READY       string = "ready"       // 图任务已经ready，可以运行
	GRAPH_STATUS_RUNNING     string = "running"     // 图任务正在进行中
	GRAPH_STATUS_COMPLETED   string = "completed"   // 图任务已完成
	GRAPH_STATUS_FAILED      string = "failed"      // 图任务执行失败了
)

// 图的结构
type GraphInfo struct {
	GraphID string
	Status  string
}

type ErrSpec struct {
	NodeName string
	Err      error
}

type Graph struct {
	Info          GraphInfo
	GraphInput    string
	GraphOutput   string
	NodesNum      int
	LinkMap       map[int][]int
	InputManagers data.GraphInputManager
	OutputManages data.GraphOutputManager
	Nodes         []*Node

	// close graph
	refmutex *sync.Mutex
	refs     int
	stop     chan struct{}
}

type GraphManager struct {
	Mutex  *sync.Mutex
	Graphs map[string]*Graph
}

var gm = GraphManager{
	Mutex:  &sync.Mutex{},
	Graphs: make(map[string]*Graph),
}

func InitialGraph(graphfile string) (*Graph, error) {
	gc, err := graph.GetGraphConfig(graphfile)
	if err != nil {
		return nil, err
	}

	gr := Graph{
		Info: GraphInfo{
			GraphID: uuid.New().String(),
			Status:  GRAPH_STATUS_NONE,
		},
		GraphInput:    gc.Input,
		GraphOutput:   gc.Output,
		NodesNum:      gc.NodesNum,
		LinkMap:       make(map[int][]int),
		InputManagers: data.NewGraphInputManager(),
		OutputManages: data.NewGraphOutputManager(),
		refmutex:      &sync.Mutex{},
	}
	// TODO: can be read from graph
	wops := data.WindowOptions{
		Type: data.DEFAULT_WINDOW,
	}
	for id, nc := range gc.Nodes {
		n := Node{
			NodeName: nc.NodeName,
			NodeConf: nc,
		}
		n.NodeID = id + 1
		n.From = nc.InputStream.From
		switch nc.Runner {
		case design.TELEMETRY_RUNNER:
			inputHandler := node.InitializeTelemetryInputHandler(nc)
			outputHandler := node.InitializeTelemetryOutputHandler(nc)
			inputManage := data.NewInputManager(wops)
			outputManage := data.OutputManager{Output: make(chan data.DataSpec, 1000)}
			n.InputHandler = &inputHandler
			n.OutputHandler = &outputHandler
			n.Input = inputManage
			n.Output = &outputManage
			n.ExecutorStart = node.TelemetryStartExecutor
			n.ExecutorStop = node.TelemetryStopExecutor
		case design.DATA_STATISTICS_RUNNER:
			inputHandler := node.InitailizeDataStatisticsInputHandler(nc)
			outputHandler := node.InitailizeDataStatisticsOutputHandler(nc)
			inputManage := data.NewInputManager(wops)
			outputManage := data.OutputManager{Output: make(chan data.DataSpec, 1000)}
			n.InputHandler = &inputHandler
			n.OutputHandler = &outputHandler
			n.Input = inputManage
			n.Output = &outputManage
			n.ExecutorStart = node.DataStatisticsStart
			n.ExecutorStop = node.DataStatisticsStop
		case design.DATA_SUM_RUNNER:
			inputHandler := node.InitializeDataSumInputHandler(nc)
			outputHandler := node.InitializeDataSumOutputHandler(nc)
			inputManage := data.NewInputManager(wops)
			outputManage := data.OutputManager{Output: make(chan data.DataSpec, 1000)}
			n.InputHandler = &inputHandler
			n.OutputHandler = &outputHandler
			n.Input = inputManage
			n.Output = &outputManage
			n.ExecutorStart = node.DataSumStart
			n.ExecutorStop = node.DataSumStop
		}
		gr.Nodes = append(gr.Nodes, &n)
		gr.InputManagers.Mutex.Lock()
		gr.InputManagers.InMngs[n.NodeID] = n.Input
		gr.InputManagers.Mutex.Unlock()

		gr.OutputManages.Mutex.Lock()
		gr.OutputManages.OutMngs[n.NodeID] = n.Output
		gr.OutputManages.Mutex.Unlock()
	}

	// 检查LinkMap
	gr.LinkMap = gc.LinkMap
	gr.setStatusInitialized()

	gm.Mutex.Lock()
	gm.Graphs[gr.Info.GraphID] = &gr
	gm.Mutex.Unlock()
	return &gr, nil
}

func (g *Graph) GetReady() error {
	for _, n := range g.Nodes {
		if err := n.PrepareForRun(); err != nil {
			return err
		}
	}
	// 建立好图的数据通道
	for inNode, outNodes := range g.LinkMap {
		for _, outNode := range outNodes {
			if err := g.LinkNodesInputOutput(inNode, outNode); err != nil {
				return err
			}
		}
	}
	g.setStatusReady()
	return nil
}

func (g *Graph) LinkNodesInputOutput(inNodeID, outNodeID int) error {
	outMng, ok := g.OutputManages.OutMngs[inNodeID]
	if !ok {
		return fmt.Errorf("cannot find node <%d> output manage", inNodeID)
	}
	inMng, ok := g.InputManagers.InMngs[outNodeID]
	if !ok {
		return fmt.Errorf("cannot find node < %d> input manage", outNodeID)
	}
	go func() {
		for d := range outMng.Output {
			inMng.AddPacket(d)
		}
		log.R.Infof("graph <%s> data channel between (node%d) and (node%d) has been closed\n", g.Info.GraphID, inNodeID, outNodeID)
	}()
	return nil
}

func (g *Graph) Run() error {

	// TODO：判断当前graph的状态
	if g.Info.Status != GRAPH_STATUS_READY {
		return fmt.Errorf("GRAPH is not ready")
	}

	// 运行前先把停止器初始化好
	g.stop = make(chan struct{})

	// 由后往前 运行节点任务
	g.refs = g.NodesNum
	log.R.Debugf("total number of node task: %d", g.refs)
	g.setStatusRunning()
	for id := g.NodesNum - 1; id >= 0; id-- {
		n := g.Nodes[id]
		go func() {
			// node任务执行成功，程序会一直阻塞在Start()，只有任务失败才会返回
			err := n.Execute()
			if err != nil {
				// node 任务失败了，说明图初始化时存在问题，这个时候应该重新初始化图
				// 只有任务执行失败才会返回err
				g.setStatusFailed()
				log.R.Errorf("execute <%s> task failed, err: %v", n.NodeName, err)
			}
			g.refmutex.Lock()
			g.refs--
			if g.refs == 0 {
				close(g.stop)
				log.R.Debug("close graph stop channel")
			}
			g.refmutex.Unlock()
			log.R.Infof("graph (%s) node%d task exit", g.Info.GraphID, n.NodeID)
		}()
	}
	return nil
}

func (g *Graph) Pause() error {
	switch g.Info.Status {
	case GRAPH_STATUS_NONE:
	case GRAPH_STATUS_INITIALIZED:
	case GRAPH_STATUS_COMPLETED:
	case GRAPH_STATUS_FAILED:
		fallthrough
	case GRAPH_STATUS_RUNNING:
		for _, n := range g.Nodes {
			close(n.stop)
		}
	}
	<-g.stop
	log.R.Infof("graph (%s) task pause", g.Info.GraphID)
	return nil
}

func (g *Graph) Destroy() error {
	// 判断图是否可以被destroy
	switch g.Info.Status {
	case GRAPH_STATUS_NONE:
		// 初始化未完成，gm未管理graph，直接返回
		return nil
	case GRAPH_STATUS_FAILED:
		// 说明有节点任务执行失败了，先停止任务
		fallthrough
	case GRAPH_STATUS_RUNNING:
		// 停止running，但由于是进程，必须确保任务已经停止才能去删除资源
		g.Pause()
		// 任务停止，开始删除资源
		fallthrough
	case GRAPH_STATUS_READY:
		// 初始化完成，handler等句柄也已完成初始化，LinkMap已完成
		for _, n := range g.Nodes {
			n.InputHandler.Close()
			n.OutputHandler.Close()
			// 必须按顺序关闭data channel
			n.Input.Close()
			n.Output.Close()
		}
		log.R.Infof("close graph (%s) handler", g.Info.GraphID)
		fallthrough
	case GRAPH_STATUS_INITIALIZED:
		// 初始化已完成，handler初始化只进行了赋值，gm管路graph
		fallthrough
	case GRAPH_STATUS_COMPLETED:
		gm.Mutex.Lock()
		delete(gm.Graphs, g.Info.GraphID)
		gm.Mutex.Unlock()
	}
	log.R.Infof("destroy graph (%s)", g.Info.GraphID)
	return nil
}

func (g *Graph) setStatusInitialized() {
	g.Info.Status = GRAPH_STATUS_INITIALIZED
}

func (g *Graph) setStatusReady() {
	g.Info.Status = GRAPH_STATUS_READY
}

func (g *Graph) setStatusRunning() {
	g.Info.Status = GRAPH_STATUS_RUNNING
}

func (g *Graph) setStatusFailed() {
	g.Info.Status = GRAPH_STATUS_FAILED
}
