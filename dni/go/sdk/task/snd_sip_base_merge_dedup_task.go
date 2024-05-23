package task

import (
	"fmt"
	"log"

	flowmng "github.com/amianetworks/dni/sdk/flowmanager"
	"github.com/amianetworks/dni/sdk/utils"
	"github.com/mitchellh/mapstructure"
)

type SndSIPBaseMergeDedupTask struct {
	TaskName string
	Options  SndSIPBaseMergeDedupOptions
}

type SndSIPBaseMergeDedupOptions struct {
	NumStats   NumberThreshLabels `mapstructure:"num_stat"`
	ProtoStats ProtoThreshLabels  `mapstructure:"proto_stat"`
}

type NumberThreshLabels struct {
	KeyLenThreshold int      `mapstructure:"keyLenThresh"`
	RatioMin        float64  `mapstructure:"ratioMin"`
	RatioMax        float64  `mapstructure:"ratioMax"`
	Labels          []string `mapstructure:"label"`
}

type ProtoThreshLabels struct {
	RatioMin float64  `mapstructure:"ratioMin"`
	RatioMax float64  `mapstructure:"ratioMax"`
	Labels   []string `mapstructure:"label"`
}

func NewSndSIPBaseMergeDedupTask(task string, options interface{}) Task {
	var opts SndSIPBaseMergeDedupOptions
	err := mapstructure.Decode(options, &opts)
	if err != nil {
		log.Printf("[%s] options decode error:%v", task, err)
		return nil
	}
	t := &SndSIPBaseMergeDedupTask{}
	t.TaskName = task
	t.Options = opts
	return t
}

func (t *SndSIPBaseMergeDedupTask) Start(ctx *flowmng.TaskContext) error {
	//cast ctx.Inputs
	hostNicSign, ok := ctx.Inputs.Get("NIC", 0).Data.(string)
	if !ok {
		return fmt.Errorf("[%s] cast error", t.TaskName)
	}
	pinfos, ok := ctx.Inputs.Get("PACKET", 0).Data.([]map[string]uint32)
	if !ok {
		return fmt.Errorf("[%s] cast error", t.TaskName)
	}
	attackIPMergeResult, ok := ctx.Inputs.Get("SIP", 0).Data.(utils.AttackerIPMergeResult)
	if !ok {
		return fmt.Errorf("[%s] cast error", t.TaskName)
	}
	log.Printf("[%s] nic:%v", t.TaskName, hostNicSign)
	log.Printf("[%s] pakcet infos:%v", t.TaskName, len(pinfos))
	log.Printf("[%s] mergedIPs:%v", t.TaskName, attackIPMergeResult)
	attackIPs := attackIPMergeResult.AttackerIPNets
	is_rand := attackIPMergeResult.IsRand
	ipRandCountDF := attackIPMergeResult.RandIPCountDF
	//merge other info based on sip
	mergedStats := utils.SIPBaseMerge(hostNicSign, pinfos, attackIPs, is_rand, ipRandCountDF)
	//get status:sport、dport、length、proto
	//int five tupe map
	fiveTupleMap := make(map[string]map[string]string)
	for _, mergedStat := range mergedStats {
		//sport
		sport_idx := utils.GetNumStatisticScore(mergedStat.SPort.KeyCountMap, t.Options.NumStats.KeyLenThreshold, t.Options.NumStats.RatioMin, t.Options.NumStats.RatioMax)
		mergedStat.SPort.Stat = t.Options.NumStats.Labels[sport_idx]
		//dport
		dport_idx := utils.GetNumStatisticScore(mergedStat.DPort.KeyCountMap, t.Options.NumStats.KeyLenThreshold, t.Options.NumStats.RatioMin, t.Options.NumStats.RatioMax)
		mergedStat.DPort.Stat = t.Options.NumStats.Labels[dport_idx]
		//length
		length_idx := utils.GetNumStatisticScore(mergedStat.Length.KeyCountMap, t.Options.NumStats.KeyLenThreshold, t.Options.NumStats.RatioMin, t.Options.NumStats.RatioMax)
		mergedStat.Length.Stat = t.Options.NumStats.Labels[length_idx]
		//proto
		protoScoreMap := mergedStat.Proto.ProtoStatMap
		protoCountMap := mergedStat.Proto.ProtoCountMap
		protoCountSum := 0
		for _, count := range protoCountMap {
			protoCountSum += count
		}
		utils.GetProtoStatisticScore[string](protoScoreMap, protoCountMap, float64(protoCountSum), t.Options.ProtoStats.RatioMin, t.Options.ProtoStats.RatioMax, t.Options.ProtoStats.Labels)
		//dedup
		for _, ip_idx := range mergedStat.IPIdxs {
			pinfo := pinfos[ip_idx]
			var dip, sport, dport, proto, length string
			dip = utils.Uitoa(pinfo["DIP"])
			proto = fmt.Sprintf("%d", pinfo["Proto"])
			//sport/sport_stat
			if mergedStat.SPort.Stat == "centralize" {
				sport = fmt.Sprintf("%d", pinfo["SPort"])
			} else {
				sport = mergedStat.SPort.Stat
			}
			//dport/dport_stat
			if mergedStat.DPort.Stat == "centralize" {
				dport = fmt.Sprintf("%d", pinfo["DPort"])
			} else {
				dport = mergedStat.DPort.Stat
			}
			//length/length_stat
			if mergedStat.Length.Stat == "centralize" {
				length = fmt.Sprintf("%d", pinfo["Length"])
			} else {
				length = mergedStat.Length.Stat
			}
			//five tuple key
			key := mergedStat.SIP + "-" + dip + "-" + sport + "-" + dport + "-" + proto + "-" + length
			value := map[string]string{}
			value["SIP"] = mergedStat.SIP
			value["DIP"] = dip
			value["SPort"] = sport
			value["DPort"] = dport
			value["Proto"] = proto
			value["HostNicSign"] = mergedStat.HostNicSign
			value["Length"] = length
			fiveTupleMap[key] = value
		}
	}
	//create ctx.Outputs
	ctx.Outputs.Get("CIDR", 0).Data = mergedStats
	ctx.Outputs.Get("CIDR", 1).Data = fiveTupleMap
	log.Printf("[%s] merged stats:%v", t.TaskName, mergedStats)
	log.Printf("[%s] merged records:%v", t.TaskName, fiveTupleMap)
	return nil
}

func (t *SndSIPBaseMergeDedupTask) Stop() error {
	return nil
}

func init() {
	RegisterTask("SndSIPBaseMergeDedupTask", NewSndSIPBaseMergeDedupTask)
}
