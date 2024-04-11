package runner

import (
	"fmt"
	"log"

	flowmng "github.com/amianetworks/dni/sdk/flowmanager"
	"github.com/amianetworks/dni/sdk/utils"
	"github.com/mitchellh/mapstructure"
)

type PacketFeatureExecutor struct {
	RunnerName   string
	featureNames []string
}

type PacketFeatureOptions struct {
	FeatureNames []string `mapstructure:"featurenames"`
}

func NewPacketFeatureExecutor(runner string, options map[string]interface{}) Executor {
	var opts PacketFeatureOptions
	err := mapstructure.Decode(options, &opts)
	if err != nil {
		log.Printf("%s options decode error:%v", runner, err)
		return nil
	}
	e := &PacketFeatureExecutor{
		RunnerName: runner,
	}
	e.featureNames = opts.FeatureNames
	return e
}

func (e *PacketFeatureExecutor) Start(values []*flowmng.DataSpec) ([]flowmng.DataSpec, error) {
	//inputs
	pinfos, ok := values[0].Data.([]map[string]uint32)
	if !ok {
		return []flowmng.DataSpec{}, fmt.Errorf("%s cast error", e.RunnerName)
	}
	//pstats
	pstats := utils.GetFeatureStatistics(pinfos, e.featureNames)
	//create output
	outputs := make([]flowmng.DataSpec, len(pstats))
	for fid := 0; fid < len(pstats); fid++ {
		log.Printf("[%s] %s Stats:%v", e.RunnerName, e.featureNames[fid], len(pstats[fid]))
		outputs[fid] = flowmng.DataSpec{
			Data: pstats[fid],
		}
	}
	return outputs, nil
}

func (e *PacketFeatureExecutor) Stop() error {
	return nil
}
