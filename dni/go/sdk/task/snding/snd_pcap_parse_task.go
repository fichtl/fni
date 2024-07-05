package snding

import (
	"fmt"
	"log"

	flowmng "github.com/amianetworks/dni/sdk/flowmanager"
	"github.com/amianetworks/dni/sdk/task"
	"github.com/amianetworks/dni/sdk/utils"
)

type SndPcapParseTask struct {
	TaskName string
}

func NewSndPcapParseTask(task string, options interface{}) task.Task {
	return &SndPcapParseTask{
		TaskName: task,
	}
}

func (t *SndPcapParseTask) Open(ctx *flowmng.TaskContext) error {
	return nil
}

func (t *SndPcapParseTask) Process(ctx *flowmng.TaskContext) error {
	fin, ok := ctx.Inputs.Get("PATH", 0).Data.(string)
	if !ok {
		return fmt.Errorf("[%s] cast error", t.TaskName)
	}
	pinfos, err := utils.GetPacketInfos(fin)
	if err != nil {
		return fmt.Errorf("[%s] get packet info error:%v", t.TaskName, err)
	}
	ctx.Outputs.Get("", 0).Data = pinfos
	log.Printf("[%s] packet infos:%v", t.TaskName, pinfos)
	return nil
}

func (t *SndPcapParseTask) Close() error {
	return nil
}

func init() {
	task.RegisterTask("SndPcapParseTask", NewSndPcapParseTask)
}
