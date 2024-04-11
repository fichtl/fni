package runner

import (
	"fmt"
	"log"

	flowmng "github.com/amianetworks/dni/sdk/flowmanager"
	"github.com/amianetworks/dni/sdk/utils"
	"github.com/mitchellh/mapstructure"
)

type ThresholdExecutor struct {
	RunnerName string
	Options    ThresholdOptions
}

type ThresholdOptions struct {
	Thresholds []float64 `mapstructure:"thresholds"`
	Scores     []float64 `mapstructure:"scores"`
}

func NewThresholdExecutor(runner string, options map[string]interface{}) Executor {
	var opts ThresholdOptions
	err := mapstructure.Decode(options, &opts)
	if err != nil {
		log.Printf("[%s] options decode error:%v", runner, err)
		return nil
	}
	e := &ThresholdExecutor{}
	e.RunnerName = runner
	e.Options = opts
	return e
}

func (e *ThresholdExecutor) Start(values []*flowmng.DataSpec) ([]flowmng.DataSpec, error) {
	val, ok := values[0].Data.(float64)
	if !ok {
		return []flowmng.DataSpec{}, fmt.Errorf("[%s] cast error", e.RunnerName)
	}
	score := utils.GetThresholdScore(val, e.Options.Thresholds, e.Options.Scores)
	log.Printf("[%s] thresholds:%v", e.RunnerName, e.Options.Thresholds)
	log.Printf("[%s] scores:%v", e.RunnerName, e.Options.Scores)
	log.Printf("[%s] input value:%f score:%f", e.RunnerName, val, score)
	d := flowmng.DataSpec{
		Data: score,
	}
	return []flowmng.DataSpec{d}, nil
}

func (e *ThresholdExecutor) Stop() error {
	return nil
}
