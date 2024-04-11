package runner

import (
	"fmt"
	"log"

	flowmng "github.com/amianetworks/dni/sdk/flowmanager"
	"github.com/amianetworks/dni/sdk/utils"
	"github.com/mitchellh/mapstructure"
)

type AttackIPMergeExecutor struct {
	RunnerName string
	Options    AttackIPMergeOptions
}

type AttackIPMergeOptions struct {
	IPFw4CountRatio      float64 `mapstructure:"ipFw4CountRatio"`
	IPFw3CountRatio      float64 `mapstructure:"ipFw3CountRatio"`
	IPFw2CountRatio      float64 `mapstructure:"ipFw2CountRatio"`
	IPRandCountRatio     float64 `mapstructure:"ipRandCountRatio"`
	IPSegCoverThreshold  int     `mapstructure:"ipSegCoverThreshold"`
	IPRandCountThreshold int     `mapstructure:"ipRandCountThreshold"`
}

func NewAttackIPMergeExecutor(runner string, options map[string]interface{}) Executor {
	var opts AttackIPMergeOptions
	err := mapstructure.Decode(options, &opts)
	if err != nil {
		log.Printf("%s options decode error:%v", runner, err)
		return nil
	}
	e := &AttackIPMergeExecutor{}
	e.RunnerName = runner
	e.Options = opts
	return e
}

func (e *AttackIPMergeExecutor) Start(values []*flowmng.DataSpec) ([]flowmng.DataSpec, error) {
	ipCountDF, ok := values[0].Data.(map[uint32]int)
	if !ok {
		return []flowmng.DataSpec{}, fmt.Errorf("%s cast error", e.RunnerName)
	}
	ipCountSum := float64(0)
	for _, value := range ipCountDF {
		ipCountSum += float64(value)
	}
	log.Printf("[%s] ipCountDF:%v", e.RunnerName, ipCountDF)
	log.Printf("[%s] ipCountSum:%v", e.RunnerName, ipCountSum)
	normal_ips, ok := values[1].Data.(map[uint32]struct{})
	if !ok {
		return []flowmng.DataSpec{}, fmt.Errorf("%s cast error", e.RunnerName)
	}
	//IP Suspect FW4
	ipFw4NumThreshold := ipCountSum * e.Options.IPFw4CountRatio
	attackIPNets := utils.IPSuspectFW4(ipCountDF, int(ipFw4NumThreshold))
	log.Printf("[%s] ipfw4:%v", e.RunnerName, attackIPNets)
	// log.Printf("[%s] ipCountDF1:%v", e.RunnerName, ipCountDF)
	//IP Suspect Fw3
	ipFw3NumThreshold := ipCountSum * e.Options.IPFw3CountRatio
	attackIPFw3Nets := utils.IPSuspectFW3(ipCountDF, int(ipFw3NumThreshold), e.Options.IPSegCoverThreshold)
	attackIPNets = append(attackIPNets, attackIPFw3Nets...)
	log.Printf("[%s] ipfw3:%v", e.RunnerName, attackIPFw3Nets)
	// log.Printf("[%s] ipCountDF2:%v", e.RunnerName, ipCountDF)
	//IP Suspect Fw2
	ipFw2NumThreshold := ipCountSum * e.Options.IPFw2CountRatio
	attackIPFw2Nets := utils.IPSuspectFW2(ipCountDF, int(ipFw2NumThreshold), e.Options.IPSegCoverThreshold)
	attackIPNets = append(attackIPNets, attackIPFw2Nets...)
	log.Printf("[%s] ipfw2:%v", e.RunnerName, attackIPFw2Nets)
	// log.Printf("[%s] ipCountDF3:%v", e.RunnerName, ipCountDF)
	//judge random
	ipRandNumThreshold := ipCountSum * e.Options.IPRandCountRatio
	is_rand := utils.IPSuspectRand(normal_ips, ipCountDF, e.Options.IPRandCountThreshold, int(ipRandNumThreshold))
	//cretae outputs
	d1 := flowmng.DataSpec{
		Data: attackIPNets,
	}
	d2 := flowmng.DataSpec{
		Data: is_rand,
	}
	d3 := flowmng.DataSpec{}
	if is_rand {
		d3.Data = ipCountDF
	} else {
		d3.Data = make(map[uint32]int)
	}
	// log.Printf("[%s] ipCountDF4(random ips):%v", e.RunnerName, ipCountDF)
	log.Printf("[%s] is random:%v", e.RunnerName, is_rand)
	return []flowmng.DataSpec{d1, d2, d3}, nil
}

func (e *AttackIPMergeExecutor) Stop() error {
	return nil
}
