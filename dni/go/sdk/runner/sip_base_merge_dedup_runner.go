package runner

import (
	"fmt"
	"log"
	"net"

	flowmng "github.com/amianetworks/dni/sdk/flowmanager"
	"github.com/amianetworks/dni/sdk/utils"
	"github.com/mitchellh/mapstructure"
)

type SIPBaseMergeDedupExecutor struct {
	RunnerName string
	Options    SIPBaseMergeOptions
}

type SIPBaseMergeDedupOptions struct {
	NumTypeRatioMin   float64  `mapstructure:"numTypeRatioMin"`
	NumTypeRatioMax   float64  `mapstructure:"numTypeRatioMax"`
	ProtoTypeRatioMin float64  `mapstructure:"protoTypeRatioMin"`
	ProtoTypeRatioMax float64  `mapstructure:"protoTypeRatioMax"`
	NumStats          []string `mapstructure:"numStats"`
	PtotoStats        []string `mapstructure:"protoStats"`
}

func NewSIPBaseMergeDedupExecutor(runner string, options map[string]interface{}) Executor {
	var opts SIPBaseMergeOptions
	err := mapstructure.Decode(options, &opts)
	if err != nil {
		log.Printf("[%s] options decode error:%v", runner, err)
		return nil
	}
	e := &SIPBaseMergeDedupExecutor{}
	e.RunnerName = runner
	e.Options = opts
	return e
}

func (e *SIPBaseMergeDedupExecutor) Start(values []*flowmng.DataSpec) ([]flowmng.DataSpec, error) {
	//cast inputs
	hostNicSign, ok := values[0].Data.(string)
	if !ok {
		return []flowmng.DataSpec{}, fmt.Errorf("[%s] cast error", e.RunnerName)
	}
	pinfos, ok := values[1].Data.([]map[string]uint32)
	if !ok {
		return []flowmng.DataSpec{}, fmt.Errorf("[%s] cast error", e.RunnerName)
	}
	attackIPs, ok := values[2].Data.([]*net.IPNet)
	if !ok {
		return []flowmng.DataSpec{}, fmt.Errorf("[%s] cast error", e.RunnerName)
	}
	is_rand, ok := values[3].Data.(bool)
	if !ok {
		return []flowmng.DataSpec{}, fmt.Errorf("[%s] cast error", e.RunnerName)
	}
	ipCountDF4, ok := values[4].Data.(map[uint32]int)
	if !ok {
		return []flowmng.DataSpec{}, fmt.Errorf("[%s] cast error", e.RunnerName)
	}
	//merge other info based on sip
	mergedStats := utils.SIPBaseMerge(hostNicSign, pinfos, attackIPs, is_rand, ipCountDF4)
	//get status:sport、dport、length、proto
	//int five tupe map
	fiveTupleMap := make(map[string]map[string]string)
	for _, mergedStat := range mergedStats {
		//sport
		sport_idx := utils.GetNumStatisticScore(mergedStat.SPort.KeyCountMap, e.Options.NumTypeRatioMin, e.Options.NumTypeRatioMax)
		mergedStat.SPort.Stat = e.Options.NumStats[sport_idx]
		//dport
		dport_idx := utils.GetNumStatisticScore(mergedStat.DPort.KeyCountMap, e.Options.NumTypeRatioMin, e.Options.NumTypeRatioMax)
		mergedStat.DPort.Stat = e.Options.NumStats[dport_idx]
		//length
		length_idx := utils.GetNumStatisticScore(mergedStat.Length.KeyCountMap, e.Options.NumTypeRatioMin, e.Options.NumTypeRatioMax)
		mergedStat.Length.Stat = e.Options.NumStats[length_idx]
		//proto
		protoScoreMap := mergedStat.Proto.ProtoStatMap
		protoCountMap := mergedStat.Proto.ProtoCountMap
		utils.GetProtoStatisticScore[string](protoScoreMap, protoCountMap, e.Options.ProtoTypeRatioMin, e.Options.ProtoTypeRatioMax, e.Options.PtotoStats)
		//dedup
		for _, ip_idx := range mergedStat.IPIdxs {
			pinfo := pinfos[ip_idx]
			var dip, sport, dport, proto string
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
			//five tuple key
			key := mergedStat.SIP + "-" + dip + "-" + dport + "-" + proto
			value := map[string]string{}
			value["SIP"] = mergedStat.SIP
			value["DIP"] = dip
			value["SPort"] = sport
			value["DPort"] = dport
			value["Proto"] = proto
			value["HostNicSign"] = mergedStat.HostNicSign
			fiveTupleMap[key] = value
		}
	}
	//create outputs
	outputs := make([]flowmng.DataSpec, 1)
	outputs[0] = flowmng.DataSpec{
		Data: fiveTupleMap,
	}
	log.Printf("[%s] records:%v", e.RunnerName, fiveTupleMap)
	return outputs, nil
}

func (e *SIPBaseMergeDedupExecutor) Stop() error {
	return nil
}
