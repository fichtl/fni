package analyzer

import (
	"fmt"

	alog "github.com/amianetworks/am.modules/log"
	flowmng "github.com/amianetworks/dni/sdk/flowmanager"
	"github.com/amianetworks/dni/sdk/task"
	"github.com/amianetworks/dni/sdk/task/dms/common"
	"github.com/amianetworks/dni/sdk/task/dms/config"
	"github.com/mitchellh/mapstructure"
)

type DmsFloodAttackDetectTask struct {
	TaskName string
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
		alog.R.Errorf("[%s] options decode error:%v", task, err)
		return nil
	}
	t := &DmsFloodAttackDetectTask{}
	t.TaskName = task
	t.Options = opts
	return t
}

func (t *DmsFloodAttackDetectTask) Open(ctx *flowmng.TaskContext) error {
	return nil
}

func (t *DmsFloodAttackDetectTask) Process(ctx *flowmng.TaskContext) error {
	if ctx.Inputs.Get("ProtoDiag", 0).Data == nil {
		ctx.Outputs.Values[0].Data = make([]*Respond, 0)
		return nil
	}
	protodiag, ok := ctx.Inputs.Get("ProtoDiag", 0).Data.(*ProtoDiag)
	if !ok {
		return fmt.Errorf("[%s] Protodiag cast error", t.TaskName)
	}
	ctInfo, ok := ctx.Inputs.Get("Conntrack", 0).Data.(*common.CtInfo)
	if !ok {
		return fmt.Errorf("[%s] Conntrack cast error", t.TaskName)
	}
	responds := FloodAttackDetection(protodiag, t.Options.PPS, t.Options.Cap, ctInfo)
	if len(responds) == 0 {
		for dev := range protodiag.Stat {
			alog.R.Debugf("[%s] No flood attack on dev %s", t.TaskName, dev)
		}
	}
	for _, respond := range responds {
		alog.R.Debugf("[%s] %v", t.TaskName, respond)
	}
	ctx.Outputs.Values[0].Data = responds
	return nil
}

func (t *DmsFloodAttackDetectTask) Close() error {
	return nil
}

func init() {
	task.RegisterTask("DmsFloodAttackDetectTask", NewDmsFloodAttackDetectTask)
}
