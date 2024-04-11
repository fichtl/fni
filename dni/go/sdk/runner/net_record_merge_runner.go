package runner

import (
	"fmt"

	flowmng "github.com/amianetworks/dni/sdk/flowmanager"
	"github.com/amianetworks/dni/sdk/utils"
)

type NetRecordMergeExecutor struct {
	RunnerName string
}

func NewNetRecordMergeExecutor(runner string, options map[string]interface{}) Executor {
	return &NetRecordMergeExecutor{
		RunnerName: runner,
	}
}

func (e *NetRecordMergeExecutor) Start(values []*flowmng.DataSpec) ([]flowmng.DataSpec, error) {
	recordMergedResults := make(map[string]map[string]struct{})
	//cast records
	for idx := 0; idx < len(values); idx++ {
		mergedStats, ok := values[idx].Data.(map[string]*utils.SIPBaseMergedStat)
		if !ok {
			return []flowmng.DataSpec{}, fmt.Errorf("[%s] cast error", e.RunnerName)
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
			}
		}
	}
	d := flowmng.DataSpec{
		Data: recordMergedResults,
	}
	return []flowmng.DataSpec{d}, nil
}

func (e *NetRecordMergeExecutor) Stop() error {
	return nil
}
