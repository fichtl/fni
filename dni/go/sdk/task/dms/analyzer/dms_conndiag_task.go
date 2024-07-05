package analyzer

import (
	"fmt"

	flowmng "github.com/amianetworks/dni/sdk/flowmanager"
	"github.com/amianetworks/dni/sdk/task"
	"github.com/amianetworks/dni/sdk/task/dms/common"
)

type DmsConnDiagTask struct {
	TaskName string
}

func NewDmsConnDiagTask(task string, options interface{}) task.Task {
	t := &DmsConnDiagTask{}
	t.TaskName = task
	return t
}

func (t *DmsConnDiagTask) Open(ctx *flowmng.TaskContext) error {
	return nil
}

func (t *DmsConnDiagTask) Process(ctx *flowmng.TaskContext) error {
	tcpconns, ok := ctx.Inputs.Get("TCPConn", 0).Data.([]*common.ConnInfo)
	if !ok {
		return fmt.Errorf("[%s] cast error", t.TaskName)
	}
	query, ok := ctx.Inputs.Get("Query", 0).Data.(*Query)
	if !ok {
		return fmt.Errorf("[%s] cast error", t.TaskName)
	}
	if query == nil {
		ctx.Outputs.Get("ConnDiag", 0).Data = nil
		return nil
	}
	conndiag := ConnAnalyser(query.Conn, tcpconns)
	//conn diag
	ctx.Outputs.Get("ConnDiag", 0).Data = conndiag
	return nil
}

func (t *DmsConnDiagTask) Close() error {
	return nil
}

func init() {
	task.RegisterTask("DmsConnDiagTask", NewDmsConnDiagTask)
}
