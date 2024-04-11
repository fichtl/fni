package runner

import (
	"fmt"
	"log"
	"math"

	flowmng "github.com/amianetworks/dni/sdk/flowmanager"
	"github.com/amianetworks/dni/sdk/utils"
	"github.com/mitchellh/mapstructure"
)

type ProtoStatisticExecutor struct {
	RunnerName string
	Options    ProtoStatisticOptions
}

type ProtoStatisticOptions struct {
	RatioMin float64   `mapstructure:"ratiomin"`
	RatioMax float64   `mapstructure:"ratiomax"`
	Scores   []float64 `mapstructure:"scores"`
}

func NewProtoStatisticExecutor(runner string, options map[string]interface{}) Executor {
	var opts ProtoStatisticOptions
	err := mapstructure.Decode(options, &opts)
	if err != nil {
		log.Printf("[%s] options decode error:%v", runner, err)
		return nil
	}
	e := &ProtoStatisticExecutor{}
	e.RunnerName = runner
	e.Options = opts
	return e
}

func (e *ProtoStatisticExecutor) Start(values []*flowmng.DataSpec) ([]flowmng.DataSpec, error) {
	protoFeatureMap, ok := values[0].Data.(map[uint32]int)
	if !ok {
		return []flowmng.DataSpec{}, fmt.Errorf("[%s] cast error", e.RunnerName)
	}
	protoScoreMap := make(map[uint32]float64)
	utils.GetProtoStatisticScore[float64](protoScoreMap, protoFeatureMap, e.Options.RatioMin, e.Options.RatioMax, e.Options.Scores)
	var maxscore float64
	for _, protoscore := range protoScoreMap {
		maxscore = math.Max(protoscore, maxscore)
	}
	log.Printf("[%s] proto score map:%v", e.RunnerName, protoScoreMap)
	log.Printf("[%s] max proto score:%v", e.RunnerName, maxscore)
	output := flowmng.DataSpec{
		Data: maxscore,
	}
	return []flowmng.DataSpec{output}, nil
}

func (e *ProtoStatisticExecutor) Stop() error {
	return nil
}
