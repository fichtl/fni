package runner

import (
	"fmt"
	"log"
	"time"

	flowmng "github.com/amianetworks/dni/sdk/flowmanager"
)

type MultiInOutTestExecutor struct {
	RunnerName string
}

func NewMultiOutTestExecutor(runner string, options map[string]interface{}) Executor {
	e := &MultiInOutTestExecutor{
		RunnerName: runner,
	}
	return e
}

func (e *MultiInOutTestExecutor) Start(values []*flowmng.DataSpec) ([]flowmng.DataSpec, error) {
	outputs := make([]flowmng.DataSpec, 2)
	//数据类型转换
	a, ok := values[0].Data.(int)
	if !ok {
		return outputs, fmt.Errorf("multi_in_out_test_executor: cast error")
	}
	b, ok := values[1].Data.(int)
	if !ok {
		return outputs, fmt.Errorf("multi_in_out_test_executor: cast error")
	}
	//计算
	outputs[0] = flowmng.DataSpec{
		Type:      flowmng.DATA_TYPE_INT,
		TimeStamp: time.Now(),
		Data:      a - b,
	}
	outputs[1] = flowmng.DataSpec{
		Type:      flowmng.DATA_TYPE_INT,
		TimeStamp: time.Now(),
		Data:      a + b,
	}
	log.Printf("multi_in_out_test_res1:%d", a-b)
	log.Printf("multi_in_out_test_res2:%d", a+b)
	return outputs, nil
}

func (e *MultiInOutTestExecutor) Stop() error {
	return nil
}
