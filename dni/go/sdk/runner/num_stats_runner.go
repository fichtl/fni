package runner

import (
	"fmt"
	"log"

	flowmng "github.com/amianetworks/dni/sdk/flowmanager"
	"github.com/amianetworks/dni/sdk/utils"
	"github.com/mitchellh/mapstructure"
)

type NumberStatisticExecutor struct {
	RunnerName string
	Options    NumberStatisticOptions
}

type NumberStatisticOptions struct {
	RatioMin float64   `mapstructure:"ratiomin"`
	RatioMax float64   `mapstructure:"ratiomax"`
	Scores   []float64 `mapstructure:"scores"`
}

func NewNumberStatisticExecutor(runner string, options map[string]interface{}) Executor {
	var opts NumberStatisticOptions
	err := mapstructure.Decode(options, &opts)
	if err != nil {
		log.Printf("[%s] options decode error:%v", runner, err)
		return nil
	}
	e := &NumberStatisticExecutor{}
	e.RunnerName = runner
	e.Options = opts
	return e
}

func (e *NumberStatisticExecutor) Start(values []*flowmng.DataSpec) ([]flowmng.DataSpec, error) {
	numFeatureMap, ok := values[0].Data.(map[uint32]int)
	if !ok {
		return []flowmng.DataSpec{}, fmt.Errorf("[%s] cast error", e.RunnerName)
	}
	//numKeyLen
	numKeyLen := len(numFeatureMap)
	numValueSum := float64(0)
	numKeys := make([]uint32, 0)
	for key, value := range numFeatureMap {
		numKeys = append(numKeys, key)
		numValueSum += float64(value)
	}
	//keyDiffSeriesTypeNum
	keyDiffSeries := utils.GetDiff(numKeys)
	keyDiffSeriesTypeNum := utils.GetTypeNum[uint32](keyDiffSeries)
	//get score idx
	var score float64
	condthrehold := numValueSum * e.Options.RatioMin
	if numKeyLen < int(condthrehold) {
		score = e.Options.Scores[0]
	} else {
		if keyDiffSeriesTypeNum <= int(numValueSum*e.Options.RatioMin) {
			score = e.Options.Scores[1]
		} else if keyDiffSeriesTypeNum >= int(numValueSum*e.Options.RatioMax) {
			score = e.Options.Scores[2]
		} else {
			score = e.Options.Scores[3]
		}
	}
	log.Printf("[%s] numKeyLen:%v", e.RunnerName, numKeyLen)
	log.Printf("[%s] keyDiffSeries:%v", e.RunnerName, keyDiffSeries)
	log.Printf("[%s] keyDiffSeriesTypeNum:%v", e.RunnerName, keyDiffSeriesTypeNum)
	log.Printf("[%s] score:%f", e.RunnerName, score)
	d := flowmng.DataSpec{
		Data: score,
	}
	return []flowmng.DataSpec{d}, nil
}

func (e *NumberStatisticExecutor) Stop() error {
	return nil
}
