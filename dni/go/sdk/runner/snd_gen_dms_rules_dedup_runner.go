package runner

import (
	"fmt"

	flowmng "github.com/amianetworks/dni/sdk/flowmanager"
	"github.com/amianetworks/dni/sdk/utils"
)

type SndGenDMSRulesDedupExecutor struct {
	RunnerName string
}

func NewSndGenDMSRulesDedupExecutor(runner string, options map[string]interface{}) Executor {
	return &SndGenDMSRulesDedupExecutor{
		RunnerName: runner,
	}
}

func (e *SndGenDMSRulesDedupExecutor) Start(values []*flowmng.DataSpec) ([]flowmng.DataSpec, error) {
	//dms rule map
	allDmsRules := make([]utils.DMSRule, 0)
	//sipbasemergeresult、netdev group
	group_num := len(values) / 2
	for idx := 0; idx < group_num; idx++ {
		fiveTupleMap, ok := values[idx].Data.(map[string]map[string]string)
		if !ok {
			return []flowmng.DataSpec{}, fmt.Errorf("[%s] cast error", e.RunnerName)
		}
		netdev, ok := values[idx+1].Data.([]float64)
		if !ok {
			return []flowmng.DataSpec{}, fmt.Errorf("[%s] cast error", e.RunnerName)
		}
		//gen dms rules
		for _, value := range fiveTupleMap {
			dmsRule := utils.GenDMSRulesDedup(value, netdev)
			allDmsRules = append(allDmsRules, dmsRule)
		}
	}
	d := flowmng.DataSpec{
		Data: allDmsRules,
	}
	return []flowmng.DataSpec{d}, nil
}

func (e *SndGenDMSRulesDedupExecutor) Stop() error {
	return nil
}
