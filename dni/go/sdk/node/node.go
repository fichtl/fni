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

	InputManager          *flowmng.InputManager          // Manage node input streams
	OutputManager         *flowmng.OutputManager         // Manage node output streams
	OutputSideDataManager *flowmng.OutputSideDataManager // Manage node output sidedata
	TaskContext           *flowmng.TaskContext

	Task task.Task // Node task

	status string // Node status

	mutex    *sync.Mutex //Stop info mutex
	stopinfo bool        //Node stop info

	ErrorList []error //Node error list to store node executing error
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
	node.InputManager = flowmng.NewInputManager(unit.InputStream)
	node.OutputManager = flowmng.NewOutputManager(unit.OutputStream)
	node.OutputSideDataManager = flowmng.NewOutputSideDataManager(unit.OutputSideData)
	//context
	node.TaskContext = &flowmng.TaskContext{
		Inputs:         node.InputManager.Inputs,
		Outputs:        node.OutputManager.Outputs,
		InputSideData:  flowmng.NewDataSlice(unit.InputSideData),
		OutputSideData: node.OutputSideDataManager.Outputs,
	}
	//task
	task := task.NewTask(unit.Task, unit.Options)
	if task == nil {
		return nil, fmt.Errorf("node %d cretae %s error", nodeID, unit.Task)
	}
	node.Task = task
	node.setStatusReady()

	return &node, nil
}

func (n *Node) PrepareForRun() error {
	if err := n.Task.Open(n.TaskContext); err != nil {
		return fmt.Errorf("task open failed:%v", err)
	}
	n.OutputSideDataManager.AddAllData()
	return nil
}

func (n *Node) Execute() error {
	if err := n.statusCheckBeforExecute(); err != nil {
		return err
	}
	n.setStatusRunning()
	for {
		//input stream
		allok, allclosed := n.InputManager.Subscribe()
		if allclosed {
			//if all input channel colsed , stop node
			break
		}
		//read stop info to decide start task or not
		n.mutex.Lock()
		if n.stopinfo || !allok {
			continue
		}
		n.mutex.Unlock()

		//start task
		err := n.Task.Process(n.TaskContext)
		if err != nil {
			//if task process error,set all outputs nil
			n.TaskContext.Outputs.Reset()
			nErr := fmt.Errorf("task process error:%v", err)
			n.ErrorList = append(n.ErrorList, nErr)
		}
		//send output stream to next nodes & graph outputs
		n.OutputManager.AddAllData()
		//send output sidedata to next nodes & graph outputs
	}
	//stop executor
	if err := n.Task.Close(); err != nil {
		n.setStatusFailed()
		nErr := fmt.Errorf("node %d task stop error:%v", n.NodeID, err)
		n.ErrorList = append(n.ErrorList, nErr)
	}
	//stop output
	if err := n.OutputManager.Close(); err != nil {
		nErr := fmt.Errorf("node %d output manager stop error:%v", n.NodeID, err)
		n.ErrorList = append(n.ErrorList, nErr)
	}
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

func (n *Node) GetInputManager() *flowmng.InputManager {
	return n.InputManager
}

func (n *Node) SetNodeEdge(nextInputManagers map[string][]*flowmng.InputManager) {
	n.OutputManager.NextInputManagers = nextInputManagers
}

func (n *Node) GetInputSideData() *flowmng.DataSlice {
	return n.TaskContext.InputSideData
}

func (n *Node) SetSideDataNodeEdge(nextInputSideData map[string][]*flowmng.DataSlice) {
	n.OutputSideDataManager.NextInputSideData = nextInputSideData
}

func (n *Node) GetNodeErrorList() []error {
	nodeErrList := make([]error, 0)
	n.mutex.Lock()
	copy(nodeErrList, n.ErrorList)
	n.mutex.Unlock()
	return nodeErrList
}
