package snding

import (
	"fmt"
	"log"

	flowmng "github.com/amianetworks/dni/sdk/flowmanager"
	"github.com/amianetworks/dni/sdk/task"
	"github.com/amianetworks/dni/sdk/utils"
)

type SndNetRecordMergeTask struct {
	TaskName string
}

func NewSndNetRecordMergeTask(task string, options interface{}) task.Task {
	return &SndNetRecordMergeTask{
		TaskName: task,
	}
}

func (t *SndNetRecordMergeTask) Open(ctx *flowmng.TaskContext) error {
	return nil
}

func (t *SndNetRecordMergeTask) Process(ctx *flowmng.TaskContext) error {
	recordMergedResults := make(map[string]map[string]struct{})
	//cast records
	for idx := 0; idx < ctx.Inputs.Size; idx++ {
		mergedStats, ok := ctx.Inputs.Values[idx].Data.(map[string]*utils.SIPBaseMergedStat)
		if !ok {
			return fmt.Errorf("[%s] cast error", t.TaskName)
		}
		for sip, mergedStat := range mergedStats {
			for dip := range mergedStat.DIP.KeyCountMap {
				dip_str := utils.Uitoa(dip)
				key := sip + ":" + dip_str
				recordMergedres, ok := recordMergedResults[key]
				if !ok {
					recordMergedres = make(map[string]struct{})
				}
				recordMergedres[mergedStat.HostNicSign] = struct{}{}
				recordMergedResults[key] = recordMergedres
			}
		}
	}
	log.Printf("[%s] all ip link:%v", t.TaskName, recordMergedResults)
	ctx.Outputs.Values[0].Data = recordMergedResults
	return nil
}

func (t *SndNetRecordMergeTask) Close() error {
	return nil
}

func init() {
	task.RegisterTask("SndNetRecordMergeTask", NewSndNetRecordMergeTask)
}
