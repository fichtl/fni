package runner

import (
	"fmt"
	"time"

	"github.com/amianetworks/am.modules/log"
	flowmng "github.com/amianetworks/dni/src/flowmanager"
)

type IntSumExecutor struct {
	RunnerName   string
	LatestOutput int
}

func NewIntSumExecutor() *IntSumExecutor {
	return &IntSumExecutor{}
}

func (e *IntSumExecutor) Start(values []flowmng.DataSpec) (flowmng.DataSpec, error) {
	d := flowmng.DataSpec{
		Type: flowmng.DATA_TYPE_INT,
	}
	sum := 0
	for _, value := range values {
		switch value.Type {
		case flowmng.DATA_TYPE_INT_SLICE:
			vals := value.Data.([]int)
			for _, v := range vals {
				sum += v
			}
		case flowmng.DATA_TYPE_INT:
			val := value.Data.(int)
			sum += val
		default:
			return flowmng.DataSpec{}, fmt.Errorf("unrecognized data type (%s)", value.Type)
		}
	}
	d.Data = sum
	d.TimeStamp = time.Now()
	log.R.Debug("intsum data: ", sum)
	return d, nil
}

func (e *IntSumExecutor) Stop() error {
	return nil
}
