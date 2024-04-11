package runner

import (
	"fmt"

	flowmng "github.com/amianetworks/dni/sdk/flowmanager"
	"github.com/amianetworks/dni/sdk/utils"
)

type SndGenDMSRulesExecutor struct {
	RunnerName string
}

func NewSndGenDMSRulesExecutor(runner string, options map[string]interface{}) Executor {
	return &SndGenDMSRulesExecutor{
		RunnerName: runner,
	}
}

func (e *SndGenDMSRulesExecutor) Start(values []*flowmng.DataSpec) ([]flowmng.DataSpec, error) {
	//dms rule map
	allDmsRules := make([]utils.DMSRule, 0)
	//sipbasemergeresult、netdev group
	group_num := len(values) / 2
	for idx := 0; idx < group_num; idx++ {
		mergedStats, ok := values[idx].Data.(map[string]*utils.SIPBaseMergedStat)
		if !ok {
			return []flowmng.DataSpec{}, fmt.Errorf("[%s] cast error", e.RunnerName)
		}
		netdev, ok := values[idx].Data.([]float64)
		if !ok {
			return []flowmng.DataSpec{}, fmt.Errorf("[%s] cast error", e.RunnerName)
		}
		//gen dms rules
		for _, mergedStat := range mergedStats {
			dmsRules := utils.GenDMSRules(mergedStat, netdev)
			allDmsRules = append(allDmsRules, dmsRules...)
		}
	}
	d := flowmng.DataSpec{
		Data: allDmsRules,
	}
	return []flowmng.DataSpec{d}, nil
}

func (e *SndGenDMSRulesExecutor) Stop() error {
	return nil
}
