package node

import "fmt"

const (
	NODE_STATUS_NONE    string = "none"
	NODE_STATUS_READY   string = "ready"
	NODE_STATUS_RUNNING string = "running"
	NONE_STATUS_FAILED  string = "failed"
)

func (n *Node) setStatusNone() {
	n.status = NODE_STATUS_NONE
}

func (n *Node) setStatusReady() {
	n.status = NODE_STATUS_READY
}

func (n *Node) setStatusRunning() {
	n.status = NODE_STATUS_RUNNING
}

func (n *Node) setStatusFailed() {
	n.status = NONE_STATUS_FAILED
}

func (n *Node) GetNodeStatus() string {
	return n.status
}

func (n *Node) isStatusNone() bool {
	return n.status == NODE_STATUS_NONE
}

func (n *Node) isStatusReady() bool {
	return n.status == NODE_STATUS_READY
}

func (n *Node) isStatusRunning() bool {
	return n.status == NODE_STATUS_RUNNING
}

func (n *Node) isStatusFailed() bool {
	return n.status == NONE_STATUS_FAILED
}

func (n *Node) statusCheckBeforExecute() error {
	switch n.status {
	case NODE_STATUS_NONE:
		return fmt.Errorf("node(id:%d) is not ready", n.NodeID)
	case NODE_STATUS_READY:
		return nil
	case NODE_STATUS_RUNNING:
		return fmt.Errorf("node(id:%d) is already running", n.NodeID)
	case NONE_STATUS_FAILED:
		return fmt.Errorf("node(id:%d) initialization error, please re-initialize", n.NodeID)
	default:
		return fmt.Errorf("node(id:%d) unrecognized state (%s)", n.NodeID, n.status)
	}
}
