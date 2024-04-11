package runner

import (
	"fmt"
	"log"
	"time"

	flowmng "github.com/amianetworks/dni/sdk/flowmanager"
)

type SumExecutor struct {
	RunnerName string
}

func NewSumExecutor(runner string, options map[string]interface{}) Executor {
	return &SumExecutor{
		RunnerName: runner,
	}
}

func (e *SumExecutor) Start(values []*flowmng.DataSpec) ([]flowmng.DataSpec, error) {
	d := flowmng.DataSpec{
		Type: flowmng.DATA_TYPE_INT,
	}
	var sum float64
	for _, value := range values {
		val, ok := value.Data.(float64)
		if !ok {
			return []flowmng.DataSpec{}, fmt.Errorf("intsumexecutor cast error")
		}
		sum += val
	}
	d.Data = sum
	d.TimeStamp = time.Now()
	log.Printf("intsum data: %f", sum)
	return []flowmng.DataSpec{d}, nil
}

func (e *SumExecutor) Stop() error {
	return nil
}
