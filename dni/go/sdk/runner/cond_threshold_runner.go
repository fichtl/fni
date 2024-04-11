package runner

import (
	"fmt"
	"log"

	flowmng "github.com/amianetworks/dni/sdk/flowmanager"
	"github.com/amianetworks/dni/sdk/utils"
	"github.com/mitchellh/mapstructure"
)

type ConditionThresholdExecutor struct {
	RunnerName string
	Options    ConditionThresholdOptions
}

type ConditionThresholdOptions struct {
	CondThreshold float64   `mapstructure:"condthreshold"`
	Thresholds    []float64 `mapstructure:"thresholds"`
	Scores        []float64 `mapstructure:"scores"`
}

func NewConditionThresholdExecutor(runner string, options map[string]interface{}) Executor {
	var opts ConditionThresholdOptions
	err := mapstructure.Decode(options, &opts)
	if err != nil {
		log.Printf("[%s] options decode error:%v", runner, err)
		return nil
	}
	e := &ConditionThresholdExecutor{}
	e.RunnerName = runner
	e.Options = opts
	return e
}

func (e *ConditionThresholdExecutor) Start(values []*flowmng.DataSpec) ([]flowmng.DataSpec, error) {
	//get inputs
	val, ok := values[0].Data.(float64)
	if !ok {
		return []flowmng.DataSpec{}, fmt.Errorf("[%s] cast error", e.RunnerName)
	}
	condval, ok := values[1].Data.(float64)
	if !ok {
		return []flowmng.DataSpec{}, fmt.Errorf("[%s] cast error", e.RunnerName)
	}
	//get outputs
	score := utils.GetCondThresholdScoreSnd(val, condval, e.Options.CondThreshold, e.Options.Thresholds, e.Options.Scores)
	d := flowmng.DataSpec{
		Data: score,
	}
	log.Printf("[%s] condthreshold:%v", e.RunnerName, e.Options.CondThreshold)
	log.Printf("[%s] thresholds:%v", e.RunnerName, e.Options.Thresholds)
	log.Printf("[%s] scores:%v", e.RunnerName, e.Options.Scores)
	log.Printf("[%s] input value:%f condition value:%f score:%f", e.RunnerName, val, condval, score)
	return []flowmng.DataSpec{d}, nil
}

func (e *ConditionThresholdExecutor) Stop() error {
	return nil
}
