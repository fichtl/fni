package analyzer

import (
	"fmt"
	"log"

	flowmng "github.com/amianetworks/dni/sdk/flowmanager"
	"github.com/amianetworks/dni/sdk/task"
	"github.com/amianetworks/dni/sdk/task/dms/common"
	"github.com/amianetworks/dni/sdk/task/dms/config"
	"github.com/mitchellh/mapstructure"
)

type DmsFloodAttackDetectTask struct {
	TaskName string
	NicIP    map[string]string
	Options  DmsFloodAttackDetectOptions
}

type DmsFloodAttackDetectOptions struct {
	PPS config.ProtoThreshold `mapstructure:"pps"`
	Cap config.Capability     `mapstructure:"capability"`
}

func NewDmsFloodAttackDetectTask(task string, options interface{}) task.Task {
	var opts DmsFloodAttackDetectOptions
	err := mapstructure.Decode(options, &opts)
	if err != nil {
		log.Printf("[%s] options decode error:%v", task, err)
		return nil
	}
	t := &DmsFloodAttackDetectTask{}
	t.TaskName = task
	t.Options = opts
	return t
}

func (t *DmsFloodAttackDetectTask) Open(ctx *flowmng.TaskContext) error {
	nicIP, ok := ctx.InputSideData.Get("NicIP", 0).Data.(map[string]string)
	if !ok {
		return fmt.Errorf("[%s] cast error", t.TaskName)
	}
	t.NicIP = nicIP
	log.Printf("[%s] input side data(%s):%v", t.TaskName, "NicIP", nicIP)
	return nil
}

func (t *DmsFloodAttackDetectTask) Process(ctx *flowmng.TaskContext) error {
	protodiag, ok := ctx.Inputs.Get("ProtoDiag", 0).Data.(*ProtoDiag)
	if !ok {
		return fmt.Errorf("[%s] Protodiag cast error", t.TaskName)
	}
	ctInfo, ok := ctx.Inputs.Get("Conntrack", 0).Data.(*common.CtInfo)
	if !ok {
		log.Printf("[%s] Conntrack is %v", t.TaskName, ctInfo)
		return fmt.Errorf("[%s] Conntrack cast error", t.TaskName)
	}
	log.Printf("[%s] Protodiag:%v", t.TaskName, protodiag)
	responds := FloodAttackDetection(protodiag, t.Options.PPS, t.Options.Cap, ctInfo)
	ctx.Outputs.Get("RespondList", 0).Data = responds
	return nil
}

func (t *DmsFloodAttackDetectTask) Close() error {
	return nil
}

func init() {
	task.RegisterTask("DmsFloodAttackDetectTask", NewDmsFloodAttackDetectTask)
}
