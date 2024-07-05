package assessor

import (
	"fmt"

	alog "github.com/amianetworks/am.modules/log"
	flowmng "github.com/amianetworks/dni/sdk/flowmanager"
	"github.com/amianetworks/dni/sdk/task"
	config "github.com/amianetworks/dni/sdk/task/dms/config"
	"github.com/mitchellh/mapstructure"
)

type DmsAssessTask struct {
	TaskName string
	Options  DmsAssessOptions
}

type DmsAssessOptions struct {
	CPU       config.CPUUtilThreshold     `mapstructure:"cpu"`
	BW        config.BwUtilThreshold      `mapstructure:"bw"`
	TCPConn   config.TcpConnUtilThreshold `mapstructure:"tcpconn"`
	PPS       config.ProtoThreshold       `mapstructure:"pps"`
	Frequency int                         `mapstructure:"frequency"`
}

func NewDmsAssessTask(task string, options interface{}) task.Task {
	var opts DmsAssessOptions
	err := mapstructure.Decode(options, &opts)
	if err != nil {
		alog.R.Errorf("[%s] options decode error:%v", task, err)
		return nil
	}
	t := &DmsAssessTask{}
	t.TaskName = task
	t.Options = opts
	return t
}

func (t *DmsAssessTask) Open(ctx *flowmng.TaskContext) error {
	return nil
}

func (t *DmsAssessTask) Process(ctx *flowmng.TaskContext) error {
	assessorData, ok := ctx.Inputs.Get("STAT", 0).Data.(AssessorData)
	if !ok {
		return fmt.Errorf("[%s] cast error", t.TaskName)
	}
	//res
	assessorInd := new(AssessorInd)
	//cpu
	cpuStatus := CalcCPUUtil(assessorData.CPU, t.Options.CPU)
	alog.R.Debugf("[%s] CPU detailed:%f,status:%v", t.TaskName, assessorData.CPU, cpuStatus)
	//bw
	bwStatus, bwStatusMap := CalcBWUtil(assessorData.BW, assessorData.NicSpeed, t.Options.BW)
	alog.R.Debugf("[%s] Bandwidth detailed:%v status:%v", t.TaskName, assessorData.BW, bwStatusMap)
	//tcpconn
	tcpConnStatus := CalcTCPConn(assessorData.TCPConn, t.Options.TCPConn)
	alog.R.Debugf("[%s] TCP Connection detailed:%v,status:%v", t.TaskName, assessorData.TCPConn, tcpConnStatus)
	//snmp
	protoProhibit := make(map[string]bool)
	snmpStatus := CalcSnmpUtil(assessorData.SNMP, uint64(t.Options.Frequency), t.Options.PPS, protoProhibit)
	alog.R.Debugf("[%s] SNMP detailed:%v,status:%v", t.TaskName, assessorData.SNMP, snmpStatus)
	//create outputs
	assessorInd.CPU = cpuStatus
	assessorInd.BW = bwStatus
	assessorInd.PerNicBW = bwStatusMap
	assessorInd.TCPConn = tcpConnStatus
	assessorInd.SNMP = snmpStatus
	ctx.Outputs.Get("", 0).Data = assessorInd
	return nil
}

func (t *DmsAssessTask) Close() error {
	return nil
}

func init() {
	task.RegisterTask("DmsAssessTask", NewDmsAssessTask)
}
