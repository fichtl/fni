package analyzer

import (
	"fmt"

	alog "github.com/amianetworks/am.modules/log"
	flowmng "github.com/amianetworks/dni/sdk/flowmanager"
	"github.com/amianetworks/dni/sdk/task"
	"github.com/amianetworks/dni/sdk/task/dms/config"
	"github.com/mitchellh/mapstructure"
)

type DmsSlowAttackDetectTask struct {
	TaskName string
	NicIP    map[string]string
	Options  DmsSlowAttackDetectOptions
}

type DmsSlowAttackDetectOptions struct {
	TCPConn config.TcpConnUtilThreshold `mapstructure:"tcpconn"`
}

func NewDmsSlowAttackDetectTask(task string, options interface{}) task.Task {
	var opts DmsSlowAttackDetectOptions
	err := mapstructure.Decode(options, &opts)
	if err != nil {
		alog.R.Errorf("[%s] options decode error:%v", task, err)
		return nil
	}
	t := &DmsSlowAttackDetectTask{}
	t.TaskName = task
	t.Options = opts
	return t
}

func (t *DmsSlowAttackDetectTask) Open(ctx *flowmng.TaskContext) error {
	return nil
}

func (t *DmsSlowAttackDetectTask) Process(ctx *flowmng.TaskContext) error {
	if ctx.Inputs.Get("ConnDiag", 0).Data == nil {
		ctx.Outputs.Values[0].Data = nil
		return nil
	}
	conndiag, ok := ctx.Inputs.Get("ConnDiag", 0).Data.(*ConnDiag)
	if !ok {
		return fmt.Errorf("[%s] cast error", t.TaskName)
	}
	//nic ip
	nicIP, ok := ctx.Inputs.Get("NicIP", 0).Data.(map[string]string)
	if !ok {
		return fmt.Errorf("[%s] cast error", t.TaskName)
	}
	respond := SlowHttpDetection(conndiag, t.Options.TCPConn, nicIP)
	if respond == nil {
		alog.R.Debugf("[%s] No slow attack", t.TaskName)
	} else {
		alog.R.Debugf("[%s] %v", t.TaskName, respond)
	}
	ctx.Outputs.Values[0].Data = respond
	return nil
}

func (t *DmsSlowAttackDetectTask) Close() error {
	return nil
}

func init() {
	task.RegisterTask("DmsSlowAttackDetectTask", NewDmsSlowAttackDetectTask)
}
