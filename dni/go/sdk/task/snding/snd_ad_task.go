package snding

import (
	"fmt"
	"log"

	flowmng "github.com/amianetworks/dni/sdk/flowmanager"
	"github.com/amianetworks/dni/sdk/task"
)

type SndAdTask struct {
	TaskName string
}

func NewSndAdTask(task string, options interface{}) task.Task {
	t := &SndAdTask{}
	t.TaskName = task
	return t
}

func (t *SndAdTask) Open(ctx *flowmng.TaskContext) error {
	return nil
}

func (t *SndAdTask) Process(ctx *flowmng.TaskContext) error {
	score1, ok := ctx.Inputs.Get("PACKET", 0).Data.(float64)
	if !ok {
		return fmt.Errorf("[%s] cast error", t.TaskName)
	}
	score2, ok := ctx.Inputs.Get("NETDEV", 0).Data.(float64)
	if !ok {
		return fmt.Errorf("[%s] cast error", t.TaskName)
	}
	score3, ok := ctx.Inputs.Get("RESOURCE", 0).Data.(float64)
	if !ok {
		return fmt.Errorf("[%s] cast error", t.TaskName)
	}
	packet_score := int(score1)
	netdev_score := int(score2)
	resource_score := int(score3)
	var abnormal_res int
	if packet_score == 1 && netdev_score == 1 && resource_score == 1 {
		abnormal_res = 1
	} else {
		if packet_score == 1 && netdev_score == 1 {
			abnormal_res = 2
		} else if packet_score == 1 && resource_score == 1 {
			abnormal_res = 3
		} else if netdev_score == 1 && resource_score == 1 {
			abnormal_res = 4
		}
	}
	ctx.Outputs.Get("", 0).Data = abnormal_res
	log.Printf("[%s] packet:%d netdev:%d resource:%d ,abnormal:%v", t.TaskName, packet_score, netdev_score, resource_score, abnormal_res)
	return nil
}

func (t *SndAdTask) Close() error {
	return nil
}

func init() {
	task.RegisterTask("SndAdTask", NewSndAdTask)
}
