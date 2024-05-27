package task

import (
	"fmt"
	"log"

	flowmng "github.com/amianetworks/dni/sdk/flowmanager"
	"github.com/amianetworks/dni/sdk/utils"
	"github.com/mitchellh/mapstructure"
)

type SndAttackerIPMergeTask struct {
	TaskName string
	Options  SndAttackerIPMergeOptions
}

type SndAttackerIPMergeOptions struct {
	IPFw4CountRatio      float64 `mapstructure:"ipFw4CountRatio"`
	IPFw3CountRatio      float64 `mapstructure:"ipFw3CountRatio"`
	IPFw2CountRatio      float64 `mapstructure:"ipFw2CountRatio"`
	IPRandCountRatio     float64 `mapstructure:"ipRandCountRatio"`
	IPSegCoverThreshold  int     `mapstructure:"ipSegCoverThreshold"`
	IPRandCountThreshold int     `mapstructure:"ipRandCountThreshold"`
}

func NewSndAttackerIPMergeTask(task string, options interface{}) Task {
	var opts SndAttackerIPMergeOptions
	err := mapstructure.Decode(options, &opts)
	if err != nil {
		log.Printf("%s options decode error:%v", task, err)
		return nil
	}
	t := &SndAttackerIPMergeTask{}
	t.TaskName = task
	t.Options = opts
	return t
}

func (t *SndAttackerIPMergeTask) Open(ctx *flowmng.TaskContext) error {
	return nil
}

func (t *SndAttackerIPMergeTask) Process(ctx *flowmng.TaskContext) error {
	ipCountDF, ok := ctx.Inputs.Get("SIP", 0).Data.(map[uint32]int)
	if !ok {
		return fmt.Errorf("%s cast error", t.TaskName)
	}
	ipCountSum := float64(0)
	for _, value := range ipCountDF {
		ipCountSum += float64(value)
	}
	log.Printf("[%s] ipCountDF:%v", t.TaskName, ipCountDF)
	log.Printf("[%s] ipCountSum:%v", t.TaskName, ipCountSum)
	normal_ips, ok := ctx.Inputs.Get("KNOWN_IPS", 0).Data.(map[uint32]struct{})
	if !ok {
		return fmt.Errorf("%s cast error", t.TaskName)
	}
	//IP Suspect FW4
	ipFw4NumThreshold := ipCountSum * t.Options.IPFw4CountRatio
	attackIPNets := utils.IPSuspectFW4(ipCountDF, int(ipFw4NumThreshold))
	log.Printf("[%s] ipfw4:%v", t.TaskName, attackIPNets)
	// log.Printf("[%s] ipCountDF1:%v", t.TaskName, ipCountDF)
	//IP Suspect Fw3
	ipFw3NumThreshold := ipCountSum * t.Options.IPFw3CountRatio
	attackIPFw3Nets := utils.IPSuspectFW3(ipCountDF, int(ipFw3NumThreshold), t.Options.IPSegCoverThreshold)
	attackIPNets = append(attackIPNets, attackIPFw3Nets...)
	log.Printf("[%s] ipfw3:%v", t.TaskName, attackIPFw3Nets)
	// log.Printf("[%s] ipCountDF2:%v", t.TaskName, ipCountDF)
	//IP Suspect Fw2
	ipFw2NumThreshold := ipCountSum * t.Options.IPFw2CountRatio
	attackIPFw2Nets := utils.IPSuspectFW2(ipCountDF, int(ipFw2NumThreshold), t.Options.IPSegCoverThreshold)
	attackIPNets = append(attackIPNets, attackIPFw2Nets...)
	log.Printf("[%s] ipfw2:%v", t.TaskName, attackIPFw2Nets)
	// log.Printf("[%s] ipCountDF3:%v", t.TaskName, ipCountDF)
	//judge random
	ipRandNumThreshold := ipCountSum * t.Options.IPRandCountRatio
	is_rand := utils.IPSuspectRand(normal_ips, ipCountDF, t.Options.IPRandCountThreshold, int(ipRandNumThreshold))
	//cretae ctx.Outputs
	if is_rand {
		ipCountDF = make(map[uint32]int)
	}

	attackerIPMergeResult := utils.AttackerIPMergeResult{
		AttackerIPNets: attackIPNets,
		IsRand:         is_rand,
		RandIPCountDF:  ipCountDF,
	}
	ctx.Outputs.Values[0].Data = attackerIPMergeResult
	log.Printf("[%s] attacker ipnets:%v", t.TaskName, attackIPNets)
	log.Printf("[%s] is random:%v", t.TaskName, is_rand)
	return nil
}

func (t *SndAttackerIPMergeTask) Close() error {
	return nil
}

func init() {
	RegisterTask("SndAttackerIPMergeTask", NewSndAttackerIPMergeTask)
}
