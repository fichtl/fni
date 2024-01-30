package node

import (
	flowmng "github.com/amianetworks/dni/src/flowmanager"
	"github.com/amianetworks/dni/src/graph"
	"github.com/amianetworks/dni/src/runner"
)

type Node struct {
	NodeID int    // 节点的顺序索引
	Runner string // 节点的Runner名称

	// Input Stream Info
	InputManager  *flowmng.InputManager  // 管理节点输入流
	OutputManager *flowmng.OutputManager // 管理节点输出流

	RunnerExecutor runner.Executor // 节点具体的执行器

	stop   chan struct{} // 节点执行状态的控制flag
	status string        // 节点的实时状态
}

// 初始化节点
func InitNode(nodeID int, unit graph.NodeUnit) (*Node, error) {
	node := Node{
		NodeID: nodeID,
		Runner: unit.Runner,
	}
	winOpts, err := flowmng.NewWindowOptions(unit.InputStreamInfo.Window, unit.InputStreamInfo.WinSize)
	if err != nil {
		return nil, err
	}
	node.InputManager = flowmng.NewInputManager(winOpts)
	node.OutputManager = flowmng.NewOutputManager()
	executor, err := runner.NewRunnerExecutor(unit.Runner)
	if err != nil {
		return nil, err
	}
	node.RunnerExecutor = executor
	node.setStatusReady()

	return &node, nil
}

// 节点的具体执行流程
func (n *Node) Execute() error {
	if err := n.statusCheckBeforExecute(); err != nil {
		return err
	}

	n.setStatusRunning()
	n.stop = make(chan struct{})
	for {
		select {
		case data := <-n.InputManager.Input:
			output, err := n.RunnerExecutor.Start(data)
			if err != nil {
				n.setStatusFailed()
				return err
			}
			n.OutputManager.Output <- output
		case <-n.stop:
			if err := n.RunnerExecutor.Stop(); err != nil {
				n.setStatusFailed()
				return err
			}
			n.setStatusReady()
			return nil
		}
	}
}

func (n *Node) Stop() {
	if n.isStatusFailed() || n.isStatusRunning() {
		close(n.stop)
	}
}

func (n *Node) Destroy() error {
	n.Stop()
	n.InputManager.Close()
	n.OutputManager.Close()
	return nil
}
