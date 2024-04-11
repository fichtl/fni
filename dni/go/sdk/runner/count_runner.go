package runner

import (
	"fmt"
	"log"

	flowmng "github.com/amianetworks/dni/sdk/flowmanager"
	"github.com/amianetworks/dni/sdk/utils"
	"github.com/mitchellh/mapstructure"
)

type CountExecutor struct {
	RunnerName string
	Options    CountOptions
}

type CountOptions struct {
	SpecifiedValue float64 `mapstructure:"specifiedvalue"`
}

func NewCountExecutor(runner string, options map[string]interface{}) Executor {
	var opt CountOptions
	err := mapstructure.Decode(options, &opt)
	if err != nil {
		log.Printf("%s options decode error:%v", runner, err)
		return nil
	}
	e := &CountExecutor{}
	e.Options = opt
	return e
}

func (e *CountExecutor) Start(values []*flowmng.DataSpec) ([]flowmng.DataSpec, error) {
	var count int
	for _, value := range values {
		val, ok := value.Data.(float64)
		if !ok {
			return []flowmng.DataSpec{}, fmt.Errorf("intsumexecutor cast error")
		}
		if utils.IsEqual(val, e.Options.SpecifiedValue) {
			count++
		}

	}
	d := flowmng.DataSpec{
		Data: float64(count),
	}
	log.Printf("count data=%f num: %d", e.Options.SpecifiedValue, count)
	return []flowmng.DataSpec{d}, nil
}

func (e *CountExecutor) Stop() error {
	return nil
}
