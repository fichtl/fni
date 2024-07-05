package analyzer

import (
	"fmt"

	flowmng "github.com/amianetworks/dni/sdk/flowmanager"
	"github.com/amianetworks/dni/sdk/task"
	"github.com/amianetworks/dni/sdk/task/dms/common"
	"github.com/google/gopacket"
)

type DmsProtoDiagTask struct {
	TaskName string
	IPNic    map[string]string
}

func NewDmsProtoDiagTask(task string, options interface{}) task.Task {
	t := &DmsProtoDiagTask{}
	t.TaskName = task
	return t
}

func (t *DmsProtoDiagTask) Open(ctx *flowmng.TaskContext) error {
	return nil
}

func (t *DmsProtoDiagTask) Process(ctx *flowmng.TaskContext) error {
	//query
	query, ok := ctx.Inputs.Get("Query", 0).Data.(*Query)
	if !ok {
		return fmt.Errorf("[%s] cast error", t.TaskName)
	}
	if query == nil {
		ctx.Outputs.Get("CtInfo", 0).Data = nil
		ctx.Outputs.Get("ProtoDiag", 0).Data = nil
		return nil
	}
	//packets
	allDevPackets, ok := ctx.Inputs.Get("Packet", 0).Data.(map[string][]gopacket.Packet)
	if !ok {
		return fmt.Errorf("[%s] cast error", t.TaskName)
	}
	//time
	ratios, ok := ctx.Inputs.Get("Ratio", 0).Data.(map[string]float64)
	if !ok {
		return fmt.Errorf("[%s] cast error", t.TaskName)
	}
	//listen ports
	listenInfos, ok := ctx.Inputs.Get("Listen", 0).Data.([]map[string]string)
	if !ok {
		return fmt.Errorf("[%s] cast error", t.TaskName)
	}
	//conntracks
	conntracks, ok := ctx.Inputs.Get("Conntrack", 0).Data.([]*common.ConnInfo)
	if !ok {
		return fmt.Errorf("[%s] cast error", t.TaskName)
	}
	//nic ip
	nicIP, ok := ctx.Inputs.Get("NicIP", 0).Data.(map[string]string)
	if !ok {
		return fmt.Errorf("[%s] cast error", t.TaskName)
	}
	//protodiag
	protodiag := NewProtoDiag()
	//get protodiag
	counters := make(map[string]*ProtoCounter)
	for dev, m := range query.Dp {
		if m.L2 == 0 {
			continue
		}
		protos := ParseProtoIndicator(m.L2)
		counter := NewProtoCounter(dev, protos...)
		counters[dev] = counter
	}
	//proto stats
	filter := make(map[string]struct{})
	for dev, counter := range counters {
		packets, ok := allDevPackets[dev]
		if !ok {
			continue
		}
		ip := GetAddr(dev, nicIP)
		filter[ip] = struct{}{}
		for _, packet := range packets {
			counter.Count(packet)
		}
		counter.Summarize()
		protodiag.Stat[dev] = counter.ProtoStat
		protodiag.Stat[dev].Ratio = ratios[dev]
	}
	//listen ports
	GetListenPort(protodiag, listenInfos, filter)
	//protodiag outputs
	//conns
	ct := common.NewCtInfo()
	GetCtInfoSS(ct, conntracks, filter)
	//create outputs
	ctx.Outputs.Get("CtInfo", 0).Data = ct
	ctx.Outputs.Get("ProtoDiag", 0).Data = protodiag
	return nil
}

func (t *DmsProtoDiagTask) Close() error {
	return nil
}

func init() {
	task.RegisterTask("DmsProtoDiagTask", NewDmsProtoDiagTask)
}
