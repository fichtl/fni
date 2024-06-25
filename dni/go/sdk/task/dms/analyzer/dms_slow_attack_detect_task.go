package analyzer

import (
	"fmt"
	"log"

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
		log.Printf("[%s] options decode error:%v", task, err)
		return nil
	}
	t := &DmsSlowAttackDetectTask{}
	t.TaskName = task
	t.Options = opts
	return t
}

func (t *DmsSlowAttackDetectTask) Open(ctx *flowmng.TaskContext) error {
	nicIP, ok := ctx.InputSideData.Get("NicIP", 0).Data.(map[string]string)
	if !ok {
		return fmt.Errorf("[%s] cast error", t.TaskName)
	}
	t.NicIP = nicIP
	log.Printf("[%s] input side data(%s):%v", t.TaskName, "NicIP", nicIP)
	return nil
}

func (t *DmsSlowAttackDetectTask) Process(ctx *flowmng.TaskContext) error {
	conndiag, ok := ctx.Inputs.Get("ConnDiag", 0).Data.(*ConnDiag)
	if !ok {
		return fmt.Errorf("[%s] cast error", t.TaskName)
	}
	respond := SlowHttpDetection(conndiag, t.Options.TCPConn, t.NicIP)
	ctx.Outputs.Get("Respond", 0).Data = respond
	return nil
}

func (t *DmsSlowAttackDetectTask) Close() error {
	return nil
}

func init() {
	task.RegisterTask("DmsSlowAttackDetectTask", NewDmsSlowAttackDetectTask)
}
