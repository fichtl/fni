package node

import (
	"fmt"
	"log"
	"sync"

	config "github.com/amianetworks/dni/sdk/config"
	flowmng "github.com/amianetworks/dni/sdk/flowmanager"
	"github.com/amianetworks/dni/sdk/task"
)

type Node struct {
	NodeID       int
	TaskName     string            // TaskName
	InputStream  config.StreamUnit // InputStreams
	OutputStream config.StreamUnit // OutputStreams

	InputManager  *flowmng.InputManager  // Manage node input streams
	OutputManager *flowmng.OutputManager // Manage node output streams
	TaskContext   *flowmng.TaskContext

	Task task.Task // Node task

	status string // Node status

	mutex    *sync.Mutex //Stop info mutex
	stopinfo bool        //Node stop info
}

func InitNode(nodeID int, unit config.NodeUnit) (*Node, error) {
	node := Node{
		NodeID:       nodeID,
		TaskName:     unit.Task,
		InputStream:  unit.InputStream,
		OutputStream: unit.OutputStream,
		mutex:        &sync.Mutex{},
		stopinfo:     false,
	}
	node.InputManager = flowmng.NewInputManager(node.InputStream)
	node.OutputManager = flowmng.NewOutputManager(node.OutputStream)
	//context
	node.TaskContext = &flowmng.TaskContext{
		Inputs:  node.InputManager.Inputs,
		Outputs: node.OutputManager.Outputs,
	}
	task := task.NewTask(unit.Task, unit.Options)
	if task == nil {
		return nil, fmt.Errorf("node %d cretae %s error", nodeID, unit.Task)
	}
	node.Task = task
	node.setStatusReady()

	return &node, nil
}

func (n *Node) Execute() error {
	if err := n.statusCheckBeforExecute(); err != nil {
		return err
	}
	n.setStatusRunning()
	for {
		allok, allclosed := n.InputManager.Subscribe()
		if allclosed {
			//if all input channel colsed , stop node
			break
		}
		//read stop info to decide start runner executor or not
		n.mutex.Lock()
		if n.stopinfo || !allok {
			continue
		}
		n.mutex.Unlock()
		//start executor
		err := n.Task.Start(n.TaskContext)
		if err != nil {
			log.Printf("runner error:%v", err)
		}
		n.OutputManager.AddAllData()
	}
	//stop executor
	if err := n.Task.Stop(); err != nil {
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
