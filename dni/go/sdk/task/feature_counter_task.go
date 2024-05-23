package task

import (
	"fmt"
	"log"

	flowmng "github.com/amianetworks/dni/sdk/flowmanager"
	"github.com/amianetworks/dni/sdk/utils"
	"github.com/mitchellh/mapstructure"
)

type FeatureCounterTask struct {
	TaskName string
	Options  FeatureCounterOptions
}

type FeatureCounterOptions struct {
	Features []string `mapstructure:"feature"`
}

func NewFeatureCounterTask(task string, options interface{}) Task {
	var opts FeatureCounterOptions
	err := mapstructure.Decode(options, &opts)
	if err != nil {
		log.Printf("%s options decode error:%v", task, err)
		return nil
	}
	t := &FeatureCounterTask{
		TaskName: task,
	}
	t.Options = opts
	return t
}

func (t *FeatureCounterTask) Start(ctx *flowmng.TaskContext) error {
	//inputs
	pinfos, ok := ctx.Inputs.Get("", 0).Data.([]map[string]uint32)
	if !ok {
		return fmt.Errorf("[%s] cast error", t.TaskName)
	}
	//pstats
	pstats := utils.GetFeatureStatistics(pinfos, t.Options.Features)
	//create output
	for fid := 0; fid < len(pstats); fid++ {
		log.Printf("[%s] %s Stats:%v", t.TaskName, t.Options.Features[fid], len(pstats[fid]))
		ctx.Outputs.Get(t.Options.Features[fid], 0).Data = pstats[fid]
	}
	return nil
}

func (t *FeatureCounterTask) Stop() error {
	return nil
}

func init() {
	RegisterTask("FeatureCounterTask", NewFeatureCounterTask)
}
