package abitrator

import (
	"fmt"

	alog "github.com/amianetworks/am.modules/log"
	flowmng "github.com/amianetworks/dni/sdk/flowmanager"
	"github.com/amianetworks/dni/sdk/task"
	"github.com/amianetworks/dni/sdk/task/dms/assessor"
)

type DmsArbitrateTask struct {
	TaskName string
}

func NewDmsArbitrateTask(task string, options interface{}) task.Task {
	t := &DmsArbitrateTask{}
	t.TaskName = task
	return t
}

func (t *DmsArbitrateTask) Open(ctx *flowmng.TaskContext) error {
	return nil
}

func (t *DmsArbitrateTask) Process(ctx *flowmng.TaskContext) error {
	assessorInd, ok := ctx.Inputs.Get("AssessorInd", 0).Data.(*assessor.AssessorInd)
	if !ok {
		return fmt.Errorf("[%s] cast error", t.TaskName)
	}
	query, intelli := GetQuery(assessorInd)
	//create outputs
	alog.R.Debugf("[%s] Query: %v", t.TaskName, query)
	ctx.Outputs.Get("Query", 0).Data = query
	ctx.Outputs.Get("Intelligence", 0).Data = intelli
	return nil
}

func (t *DmsArbitrateTask) Close() error {
	return nil
}

func init() {
	task.RegisterTask("DmsArbitrateTask", NewDmsArbitrateTask)
}
