package task

import (
	"fmt"
	"log"

	flowmng "github.com/amianetworks/dni/sdk/flowmanager"
	"github.com/amianetworks/dni/sdk/utils"
	"github.com/mitchellh/mapstructure"
)

type SndNumberStatsTask struct {
	TaskName string
	Options  SndNumberStatsOptions
}

type SndNumberStatsOptions struct {
	NumValueSum int       `mapstructure:"numValueSum"`
	RatioMin    float64   `mapstructure:"ratioMin"`
	RatioMax    float64   `mapstructure:"ratioMax"`
	Scores      []float64 `mapstructure:"score_thresholds"`
}

func NewSndNumberStatsTask(task string, options interface{}) Task {
	var opts SndNumberStatsOptions
	err := mapstructure.Decode(options, &opts)
	if err != nil {
		log.Printf("[%s] options decode error:%v", task, err)
		return nil
	}
	t := &SndNumberStatsTask{}
	t.TaskName = task
	t.Options = opts
	return t
}

func (t *SndNumberStatsTask) Start(ctx *flowmng.TaskContext) error {
	numFeatureMap, ok := ctx.Inputs.Get("", 0).Data.(map[uint32]int)
	if !ok {
		return fmt.Errorf("[%s] cast error", t.TaskName)
	}
	//numKeyLen
	numKeyLen := len(numFeatureMap)
	numValueSum := float64(t.Options.NumValueSum)
	numKeys := make([]uint32, 0)
	for key := range numFeatureMap {
		numKeys = append(numKeys, key)
	}
	//keyDiffSeriesTypeNum
	keyDiffSeries := utils.GetDiff(numKeys)
	keyDiffSeriesTypeNum := utils.GetTypeNum[uint32](keyDiffSeries)
	//get score idx
	var score float64
	condthrehold := numValueSum * t.Options.RatioMin
	if numKeyLen < int(condthrehold) {
		score = t.Options.Scores[0]
	} else {
		if keyDiffSeriesTypeNum <= int(numValueSum*t.Options.RatioMin) {
			score = t.Options.Scores[1]
		} else if keyDiffSeriesTypeNum >= int(numValueSum*t.Options.RatioMax) {
			score = t.Options.Scores[2]
		} else {
			score = t.Options.Scores[3]
		}
	}
	log.Printf("[%s] numKeyLen:%v", t.TaskName, numKeyLen)
	log.Printf("[%s] keyDiffSeries:%v", t.TaskName, keyDiffSeries)
	log.Printf("[%s] keyDiffSeriesTypeNum:%v", t.TaskName, keyDiffSeriesTypeNum)
	log.Printf("[%s] score:%f", t.TaskName, score)

	ctx.Outputs.Get("", 0).Data = score
	return nil
}

func (t *SndNumberStatsTask) Stop() error {
	return nil
}

func init() {
	RegisterTask("SndNumberStatsTask", NewSndNumberStatsTask)
}
