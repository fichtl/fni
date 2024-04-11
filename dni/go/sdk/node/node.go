package node

import (
	"fmt"
	"log"
	"sync"

	flowmng "github.com/amianetworks/dni/sdk/flowmanager"
	"github.com/amianetworks/dni/sdk/graph"
	"github.com/amianetworks/dni/sdk/runner"
)

type Node struct {
	NodeID       int
	Runner       string   // RunnerName
	InputStream  []string // InputStreams
	OutputStream []string // OutputStreams

	InputManager  *flowmng.InputManager  // Manage node input streams
	OutputManager *flowmng.OutputManager // Manage node output streams

	RunnerExecutor runner.Executor // Node Executor

	status string // Node status

	mutex    *sync.Mutex //Stop info mutex
	stopinfo bool        //Node stop info
}

func InitNode(nodeID int, unit graph.NodeUnit) (*Node, error) {
	node := Node{
		NodeID:       nodeID,
		Runner:       unit.Runner,
		InputStream:  unit.InputStream,
		OutputStream: unit.OutputStream,
		mutex:        &sync.Mutex{},
		stopinfo:     false,
	}
	node.InputManager = flowmng.NewInputManager(node.InputStream)
	node.OutputManager = flowmng.NewOutputManager(node.OutputStream)
	executor := runner.NewRunnerExecutor(unit.Runner, unit.RunnerOptions)
	if executor == nil {
		return nil, fmt.Errorf("node %d cretae %s error", nodeID, unit.Runner)
	}
	node.RunnerExecutor = executor
	node.setStatusReady()

	return &node, nil
}

func (n *Node) Execute() error {
	if err := n.statusCheckBeforExecute(); err != nil {
		return err
	}
	n.setStatusRunning()
	for {
		inputdatas, allclosed := n.InputManager.Subscribe()
		if allclosed {
			//if all input channel colsed , stop node
			break
		}
		//read stop info to decide start runner executor or not
		n.mutex.Lock()
		if n.stopinfo {
			continue
		}
		n.mutex.Unlock()
		//start executor
		outputs, err := n.RunnerExecutor.Start(inputdatas)
		if err != nil {
			log.Printf("runner error:%v", err)
		}
		n.OutputManager.AddAllData(outputs)
	}
	//stop executor
	if err := n.RunnerExecutor.Stop(); err != nil {
		n.setStatusFailed()
		log.Printf("node %d runner executor stop error:%v", n.NodeID, err)
	}
	//stop output
	_ = n.OutputManager.Close()
	//change node status
	n.setStatusNone()
	log.Printf("node %d destroy", n.NodeID)
	return nil
}

func (n *Node) Destroy() error {
	if n.isStatusFailed() || n.isStatusRunning() {
		n.mutex.Lock()
		n.stopinfo = true
		n.mutex.Unlock()
		return nil
	}
	return nil
}
