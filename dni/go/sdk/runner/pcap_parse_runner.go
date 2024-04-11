package runner

import (
	"fmt"
	"log"

	flowmng "github.com/amianetworks/dni/sdk/flowmanager"
	"github.com/amianetworks/dni/sdk/utils"
)

type PcapParseExecutor struct {
	RunnerName string
}

func NewPcapParseExecutor(runner string, options map[string]interface{}) Executor {
	return &PcapParseExecutor{
		RunnerName: runner,
	}
}

func (e *PcapParseExecutor) Start(values []*flowmng.DataSpec) ([]flowmng.DataSpec, error) {
	fin, ok := values[0].Data.(string)
	if !ok {
		return []flowmng.DataSpec{}, fmt.Errorf("[%s] cast error", e.RunnerName)
	}
	pinfos, err := utils.GetPacketInfos(fin)
	if err != nil {
		return []flowmng.DataSpec{}, fmt.Errorf("[%s] get packet info error:%v", e.RunnerName, err)
	}
	d := flowmng.DataSpec{
		Data: pinfos,
	}
	log.Printf("[%s] packet infos:%v", e.RunnerName, pinfos)
	return []flowmng.DataSpec{d}, nil
}

func (e *PcapParseExecutor) Stop() error {
	return nil
}
