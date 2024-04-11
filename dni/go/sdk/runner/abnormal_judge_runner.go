package runner

import (
	"fmt"
	"log"

	flowmng "github.com/amianetworks/dni/sdk/flowmanager"
)

type AbnormalJudgeExecutor struct {
	RunnerName string
	Options    CountOptions
}

func NewAbnormalJudgeExecutor(runner string, options map[string]interface{}) Executor {
	e := &AbnormalJudgeExecutor{}
	e.RunnerName = runner
	return e
}

func (e *AbnormalJudgeExecutor) Start(values []*flowmng.DataSpec) ([]flowmng.DataSpec, error) {
	score1, ok := values[0].Data.(float64)
	if !ok {
		return []flowmng.DataSpec{}, fmt.Errorf("[%s] cast error", e.RunnerName)
	}
	score2, ok := values[1].Data.(float64)
	if !ok {
		return []flowmng.DataSpec{}, fmt.Errorf("[%s] cast error", e.RunnerName)
	}
	score3, ok := values[2].Data.(float64)
	if !ok {
		return []flowmng.DataSpec{}, fmt.Errorf("[%s] cast error", e.RunnerName)
	}
	packet_score := int(score1)
	netdev_score := int(score2)
	resource_score := int(score3)
	d := flowmng.DataSpec{
		Data: 0,
	}
	if packet_score == 1 && netdev_score == 1 && resource_score == 1 {
		d.Data = 1
	} else {
		if packet_score == 1 && netdev_score == 1 {
			d.Data = 2
		} else if packet_score == 1 && resource_score == 1 {
			d.Data = 3
		} else if netdev_score == 1 && resource_score == 1 {
			d.Data = 4
		}
	}
	log.Printf("[%s] packet:%d netdev:%d resource:%d ,abnormal:%v", e.RunnerName, packet_score, netdev_score, resource_score, d.Data)
	return []flowmng.DataSpec{d}, nil
}

func (e *AbnormalJudgeExecutor) Stop() error {
	return nil
}
