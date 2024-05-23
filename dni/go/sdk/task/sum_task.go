package task

import (
	"fmt"
	"log"

	flowmng "github.com/amianetworks/dni/sdk/flowmanager"
)

type SumTask struct {
	TaskName string
}

func NewSumTask(task string, options interface{}) Task {
	return &SumTask{
		TaskName: task,
	}
}

func (t *SumTask) Start(ctx *flowmng.TaskContext) error {
	var sum float64
	for _, value := range ctx.Inputs.Values {
		val, ok := value.Data.(float64)
		if !ok {
			return fmt.Errorf("intsumexecutor cast error")
		}
		sum += val
	}
	ctx.Outputs.Values[0].Data = sum
	log.Printf("[%s] intsum data: %f", t.TaskName, sum)
	return nil
}

func (t *SumTask) Stop() error {
	return nil
}

func init() {
	RegisterTask("SumTask", NewSumTask)
}
