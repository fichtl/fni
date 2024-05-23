package task

import (
	"fmt"
	"log"

	flowmng "github.com/amianetworks/dni/sdk/flowmanager"
	"github.com/gogf/gf/v2/container/gmap"
	"github.com/gogf/gf/v2/util/gutil"
	"github.com/mitchellh/mapstructure"
)

type ThresholdTask struct {
	TaskName string
	Options  ThresholdOptions
}

type ThresholdOptions struct {
	ThresholdScores []ThresholdScore `mapstructure:"thresh_scores"`
	Default         float64          `mapstructure:"default"`
}

type ThresholdScore struct {
	Threshold float64 `mapstructure:"threshold"`
	Score     float64 `mapstructure:"score"`
}

func NewThresholdTask(task string, options interface{}) Task {
	var opts ThresholdOptions
	err := mapstructure.Decode(options, &opts)
	if err != nil {
		log.Printf("[%s] options decode error:%v", task, err)
		return nil
	}
	t := &ThresholdTask{}
	t.TaskName = task
	t.Options = opts
	return t
}

func (t *ThresholdTask) Start(ctx *flowmng.TaskContext) error {
	val, ok := ctx.Inputs.Values[0].Data.(float64)
	if !ok {
		return fmt.Errorf("[%s] cast error", t.TaskName)
	}
	//ordered map
	threshold_score_map := gmap.NewTreeMap(gutil.ComparatorFloat64, true)
	for _, threshold_score := range t.Options.ThresholdScores {
		threshold_score_map.Set(threshold_score.Threshold, threshold_score.Score)
	}
	//scores
	var score_id int = -1
	thresholds := threshold_score_map.Keys()
	for _, threshold := range thresholds {
		if val >= threshold.(float64) {
			score_id++
		} else {
			break
		}
	}
	if score_id == -1 {
		ctx.Outputs.Values[0].Data = t.Options.Default
	} else {
		ctx.Outputs.Values[0].Data = threshold_score_map.Values()[score_id]
	}
	log.Printf("[%s] thresholds & scores:%v", t.TaskName, t.Options.ThresholdScores)
	log.Printf("[%s] input value:%f score:%v", t.TaskName, val, ctx.Outputs.Values[0].Data)
	return nil
}

func (t *ThresholdTask) Stop() error {
	return nil
}

func init() {
	RegisterTask("ThresholdTask", NewThresholdTask)
}
