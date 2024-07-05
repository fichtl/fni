package task

import (
	"log"

	flowmng "github.com/amianetworks/dni/sdk/flowmanager"
)

type PropagateAndAddSideDataTask struct {
	TaskName      string
	InputSideData int
}

func NewPropagateAndAddSideDataTask(task string, options interface{}) Task {
	return &PropagateAndAddSideDataTask{
		TaskName: task,
	}
}

func (t *PropagateAndAddSideDataTask) Open(ctx *flowmng.TaskContext) error {
	log.Printf("[%s] side inputs size:%d", t.TaskName, ctx.InputSideData.Size)
	input_side_data, ok := ctx.InputSideData.Values[0].Data.(int)
	if !ok {
		log.Printf("[%s]input sidedata cast error", t.TaskName)
	}
	t.InputSideData = input_side_data
	//output side data
	log.Printf("input side data:%v", input_side_data)
	ctx.OutputSideData.Values[0].Data = input_side_data + 1
	log.Printf("output side data:%v", ctx.OutputSideData.Values[0].Data)
	return nil
}

func (t *PropagateAndAddSideDataTask) Process(ctx *flowmng.TaskContext) error {
	d, ok := ctx.Inputs.Values[0].Data.(int)
	if !ok {
		log.Printf("[%s]input stream cast error", t.TaskName)
	}
	output := d + t.InputSideData
	ctx.Outputs.Values[0].Data = output
	return nil
}

func (t *PropagateAndAddSideDataTask) Close() error {
	return nil
}

func init() {
	RegisterTask("PropagateAndAddSideDataTask", NewPropagateAndAddSideDataTask)
}
