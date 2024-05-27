package task

import (
	"fmt"
	"log"

	flowmng "github.com/amianetworks/dni/sdk/flowmanager"
	"github.com/gogf/gf/v2/container/gmap"
	"github.com/gogf/gf/v2/util/gutil"
	"github.com/mitchellh/mapstructure"
)

type ConditionThresholdTask struct {
	TaskName string
	Options  ConditionThresholdOptions
}

type ConditionThresholdOptions struct {
	Conditions      []float64        `mapstructure:"conditions"`
	ThresholdScores []ThresholdScore `mapstructure:"thresh_scores"`
	Default         float64          `mapstructure:"default"`
}

func NewConditionThresholdTask(task string, options interface{}) Task {
	var opts ConditionThresholdOptions
	err := mapstructure.Decode(options, &opts)
	if err != nil {
		log.Printf("[%s] options decode error:%v", task, err)
		return nil
	}
	t := &ConditionThresholdTask{}
	t.TaskName = task
	t.Options = opts
	return t
}

func (t *ConditionThresholdTask) Open(ctx *flowmng.TaskContext) error {
	return nil
}

func (t *ConditionThresholdTask) Process(ctx *flowmng.TaskContext) error {
	//get inputs
	val, ok := ctx.Inputs.Get("STAT", 0).Data.(float64)
	if !ok {
		return fmt.Errorf("[%s] cast error", t.TaskName)
	}
	condvals, ok := ctx.Inputs.Get("COND", 0).Data.([]float64)
	if !ok {
		return fmt.Errorf("[%s] cast error", t.TaskName)
	}
	if len(condvals) > len(t.Options.Conditions) {
		return fmt.Errorf("[%s] not enough conditions", t.TaskName)
	}
	//ordered map
	threshold_score_map := gmap.NewTreeMap(gutil.ComparatorFloat64, true)
	for _, threshold_score := range t.Options.ThresholdScores {
		threshold_score_map.Set(threshold_score.Threshold, threshold_score.Score)
	}
	//get score idx
	var cond bool = true
	for id, condval := range condvals {
		if condval < t.Options.Conditions[id] {
			cond = false
			break
		}
	}
	var score_id int = -1
	if cond {
		thresholds := threshold_score_map.Keys()
		for _, threshold := range thresholds {
			if val >= threshold.(float64) {
				score_id++
			}
		}
	}
	if score_id == -1 {
		ctx.Outputs.Get("", 0).Data = t.Options.Default
	} else {
		ctx.Outputs.Get("", 0).Data = threshold_score_map.Values()[score_id]
	}
	log.Printf("[%s] condthreshold:%v", t.TaskName, t.Options.Conditions)
	log.Printf("[%s] threshold & scores:%v", t.TaskName, t.Options.ThresholdScores)
	log.Printf("[%s] input value:%f condition value:%v score:%v", t.TaskName, val, condvals, ctx.Outputs.Values[0].Data)
	return nil
}

func (t *ConditionThresholdTask) Close() error {
	return nil
}

func init() {
	RegisterTask("ConditionThresholdTask", NewConditionThresholdTask)
}
