package snding

import (
	"fmt"
	"log"
	"math"

	flowmng "github.com/amianetworks/dni/sdk/flowmanager"
	"github.com/amianetworks/dni/sdk/task"
	"github.com/amianetworks/dni/sdk/utils"
	"github.com/mitchellh/mapstructure"
)

type SndProtocolStatsTask struct {
	TaskName string
	Options  SndProtocolStatsOptions
}

type SndProtocolStatsOptions struct {
	ProtoCountSum int       `mapstructure:"protoCountSum"`
	RatioMin      float64   `mapstructure:"ratioMin"`
	RatioMax      float64   `mapstructure:"ratioMax"`
	Scores        []float64 `mapstructure:"score_thresholds"`
}

func NewSndProtocolStatsTask(task string, options interface{}) task.Task {
	var opts SndProtocolStatsOptions
	err := mapstructure.Decode(options, &opts)
	if err != nil {
		log.Printf("[%s] options decode error:%v", task, err)
		return nil
	}
	t := &SndProtocolStatsTask{}
	t.TaskName = task
	t.Options = opts
	return t
}

func (t *SndProtocolStatsTask) Open(ctx *flowmng.TaskContext) error {
	return nil
}

func (t *SndProtocolStatsTask) Process(ctx *flowmng.TaskContext) error {
	protoFeatureMap, ok := ctx.Inputs.Get("", 0).Data.(map[uint32]int)
	if !ok {
		return fmt.Errorf("[%s] cast error", t.TaskName)
	}
	protoScoreMap := make(map[uint32]float64)
	utils.GetProtoStatisticScore[float64](protoScoreMap, protoFeatureMap, float64(t.Options.ProtoCountSum), t.Options.RatioMin, t.Options.RatioMax, t.Options.Scores)
	var maxscore float64
	for _, protoscore := range protoScoreMap {
		maxscore = math.Max(protoscore, maxscore)
	}
	log.Printf("[%s] proto score map:%v", t.TaskName, protoScoreMap)
	log.Printf("[%s] max proto score:%v", t.TaskName, maxscore)
	ctx.Outputs.Get("", 0).Data = maxscore
	return nil
}

func (t *SndProtocolStatsTask) Close() error {
	return nil
}

func init() {
	task.RegisterTask("SndProtocolStatsTask", NewSndProtocolStatsTask)
}
