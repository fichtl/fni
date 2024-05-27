package task

import (
	"fmt"
	"log"
	"math"

	flowmng "github.com/amianetworks/dni/sdk/flowmanager"
)

type MaxTask struct {
	TaskName string
}

func NewMaxTask(task string, options interface{}) Task {
	t := &MaxTask{
		TaskName: task,
	}
	return t
}

func (t *MaxTask) Open(ctx *flowmng.TaskContext) error {
	return nil
}

func (t *MaxTask) Process(ctx *flowmng.TaskContext) error {
	maxval, ok := ctx.Inputs.Values[0].Data.(float64)
	if !ok {
		return fmt.Errorf("%s cast error", t.TaskName)
	}
	for _, value := range ctx.Inputs.Values {
		val, ok := value.Data.(float64)
		if !ok {
			return fmt.Errorf("%s cast error", t.TaskName)
		}
		maxval = math.Max(val, maxval)
	}
	log.Printf("[%s] max value:%f", t.TaskName, maxval)
	ctx.Outputs.Values[0].Data = maxval
	return nil
}

func (t *MaxTask) Close() error {
	return nil
}

func init() {
	RegisterTask("MaxTask", NewMaxTask)
}
