package runner

import (
	"fmt"
	"math"

	flowmng "github.com/amianetworks/dni/sdk/flowmanager"
)

type MaxExecutor struct {
	RunnerName string
}

func NewMaxExecutor(runner string, options map[string]interface{}) Executor {
	e := &MaxExecutor{
		RunnerName: runner,
	}
	return e
}

func (e *MaxExecutor) Start(values []*flowmng.DataSpec) ([]flowmng.DataSpec, error) {
	maxval, ok := values[0].Data.(float64)
	if !ok {
		return []flowmng.DataSpec{}, fmt.Errorf("%s cast error", e.RunnerName)
	}
	for _, value := range values {
		val, ok := value.Data.(float64)
		if !ok {
			return []flowmng.DataSpec{}, fmt.Errorf("%s cast error", e.RunnerName)
		}
		maxval = math.Max(val, maxval)
	}
	d := flowmng.DataSpec{
		Data: maxval,
	}
	return []flowmng.DataSpec{d}, nil
}

func (e *MaxExecutor) Stop() error {
	return nil
}
