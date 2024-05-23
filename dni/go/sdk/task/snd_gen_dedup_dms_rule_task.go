package task

import (
	"fmt"

	flowmng "github.com/amianetworks/dni/sdk/flowmanager"
	"github.com/amianetworks/dni/sdk/utils"
)

type SndGenDedupTDMSRulesask struct {
	TaskName string
}

func NewSndGenDedupTDMSRulesask(task string, options interface{}) Task {
	return &SndGenDedupTDMSRulesask{
		TaskName: task,
	}
}

func (t *SndGenDedupTDMSRulesask) Start(ctx *flowmng.TaskContext) error {
	//dms rule map
	allDmsRules := make([]utils.DMSRule, 0)
	//sipbasemergeresult、netdev group
	group_num := ctx.Inputs.Size / 2
	for idx := 0; idx < group_num; idx++ {
		fiveTupleMap, ok := ctx.Inputs.Get("CIDR", idx).Data.(map[string]map[string]string)
		if !ok {
			return fmt.Errorf("[%s] cast error", t.TaskName)
		}
		netdev, ok := ctx.Inputs.Get("NETDEV", idx).Data.([]float64)
		if !ok {
			return fmt.Errorf("[%s] cast error", t.TaskName)
		}
		//gen dms rules
		for _, value := range fiveTupleMap {
			dmsRule := utils.GenDMSRulesDedup(value, netdev)
			allDmsRules = append(allDmsRules, dmsRule)
		}
	}
	ctx.Outputs.Get("", 0).Data = allDmsRules
	return nil
}

func (t *SndGenDedupTDMSRulesask) Stop() error {
	return nil
}

func init() {
	RegisterTask("SndGenDedupTDMSRulesask", NewSndGenDedupTDMSRulesask)
}
