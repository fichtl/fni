package utils

import (
	"math"
	"net"
	"sort"
	"strconv"
)

// threshold
func GetThresholdScore[T int | float64](val T, thresholds []T, scores []T) T {
	var idx = len(thresholds)
	for idx = 0; idx < len(thresholds); idx++ {
		if val < thresholds[idx] {
			return scores[idx]
		}
	}
	return scores[idx]
}

func GetThresholdScoreID(val float64, thresholds []float64) int {
	for idx := 0; idx < len(thresholds); idx++ {
		if val < thresholds[idx] {
			return idx
		}
	}
	return len(thresholds)
}

func GetCondThresholdScoreSnd(val, condval, condthreshold float64, thresholds, scores []float64) float64 {
	var cond bool = condval >= condthreshold
	if cond {
		if val <= thresholds[0] {
			return scores[0]
		} else if val >= thresholds[1] {
			return scores[1]
		}
	}
	return scores[2]
}

// GetDiff
func GetDiff(vals []uint32) []uint32 {
	sort.Slice(vals, func(i, j int) bool { return vals[i] < vals[j] })
	diff := make([]uint32, 0)
	for i := 0; i < len(vals)-1; i++ {
		diff = append(diff, vals[i+1]-vals[i])
	}
	return diff
}

// GetTypeNum
func GetTypeNum[T int | uint32 | string](vals []T) int {
	typemap := make(map[T]struct{})
	for _, val := range vals {
		typemap[val] = struct{}{}
	}
	return len(typemap)
}

// Feature Statistics
func GetFeatureStatistics(pinfos []map[string]uint32, featureNames []string) []map[uint32]int {
	fnum := len(featureNames)
	pstats := make([]map[uint32]int, fnum)
	for fid := 0; fid < fnum; fid++ {
		pstats[fid] = make(map[uint32]int)
	}
	for _, pinfo := range pinfos {
		for fid := 0; fid < fnum; fid++ {
			fname := featureNames[fid]
			fvalue := pinfo[fname]
			pstats[fid][fvalue]++
		}
	}
	return pstats
}

// Number Statistics
func GetNumStatisticScore(numFeatureMap map[uint32]int, condthreshold int, numTypeRatioMin, numTypeRatioMax float64) int {
	//numKeyLen
	numKeyLen := len(numFeatureMap)
	numValueSum := float64(0)
	numKeys := make([]uint32, 0)
	for key, value := range numFeatureMap {
		numKeys = append(numKeys, key)
		numValueSum += float64(value)
	}
	//keyDiffSeriesTypeNum
	keyDiffSeries := GetDiff(numKeys)
	keyDiffSeriesTypeNum := GetTypeNum[uint32](keyDiffSeries)
	//get score idx
	if numKeyLen < int(condthreshold) {
		return 0
	} else {
		if keyDiffSeriesTypeNum <= int(numValueSum*numTypeRatioMin) {
			return 1
		} else if keyDiffSeriesTypeNum >= int(numValueSum*numTypeRatioMax) {
			return 2
		} else {
			return 3
		}
	}
}

// Proto Statistics
func GetProtoStatisticScore[T string | float64](protoScoreMap map[uint32]T, protoFeatureMap map[uint32]int, protoCountSum, protoTypeRatioMin, protoTypeRatioMax float64, scores []T) {
	for proto, value := range protoFeatureMap {
		if value >= int(protoCountSum*protoTypeRatioMax) {
			protoScoreMap[proto] = scores[0]
		} else if value >= int(protoCountSum*protoTypeRatioMin) {
			protoScoreMap[proto] = scores[1]
		} else {
			protoScoreMap[proto] = scores[2]
		}
	}
}

func GetMinMax(vals []uint32) (min uint32, max uint32) {
	max = 0
	min = math.MaxUint32
	for _, val := range vals {
		if val > max {
			max = val
		}
		if val < min {
			min = val
		}
	}
	return min, max
}

func IPRangeToCIDR(start uint32, end uint32) []*net.IPNet {
	r := Range{
		start: net.IP(Uiton(start)),
		end:   net.IP(Uiton(end)),
	}
	ipnets := r.ToIPNets()
	return ipnets
}

type AttackerIPMergeResult struct {
	AttackerIPNets []*net.IPNet
	IsRand         bool
	RandIPCountDF  map[uint32]int
}

// ip merge 1
func IPSuspectFW4(ipCountDF map[uint32]int, ipNumThreshold int) []*net.IPNet {
	IPFW4 := make([]uint32, 0)
	for ip, count := range ipCountDF {
		if count >= ipNumThreshold {
			IPFW4 = append(IPFW4, ip)
		}
	}
	//delete ipfw4 ips from ipcountdf
	//create 32 mask attack ips from ipfw4
	attackIPNets := make([]*net.IPNet, 0)
	for _, ip := range IPFW4 {
		delete(ipCountDF, ip)
		//get ip net
		ipNet := Uitoin(ip, UINT32_MAX)
		attackIPNets = append(attackIPNets, ipNet)
	}
	return attackIPNets
}

// ip merge 2
func IPSuspectFW3(ipCountDF map[uint32]int, ipNumThreshold int, ipSegCoverThreshold int) []*net.IPNet {
	//get ipfw3df
	//get ip
	ipCountFw3DF := make(map[uint32]int)
	ipSusIPFw3DF := make(map[uint32][]uint32)
	for ip, count := range ipCountDF {
		ipFw3 := ip & (0xFFFFFF00)
		ipCountFw3DF[ipFw3] += count
		ips, ok := ipSusIPFw3DF[ipFw3]
		if !ok {
			ips = make([]uint32, 0)
		}
		ips = append(ips, ip)
		ipSusIPFw3DF[ipFw3] = ips
	}
	//get ipfw3 ips
	IPFW3 := make([]uint32, 0)
	if len(ipCountFw3DF) >= ipNumThreshold {
		for ip, count := range ipCountFw3DF {
			if count > ipNumThreshold/2 {
				IPFW3 = append(IPFW3, ip)
			}
		}
	} else {
		for ip, count := range ipCountFw3DF {
			if count > ipNumThreshold {
				IPFW3 = append(IPFW3, ip)
			}
		}
	}
	//merge
	attackIPNets := make([]*net.IPNet, 0)
	for _, ipfw3 := range IPFW3 {
		//delete ips from map
		for _, ip := range ipSusIPFw3DF[ipfw3] {
			delete(ipCountDF, ip)
		}
		//merge ip in different condition
		ipList := ipSusIPFw3DF[ipfw3]
		ipMin, ipMax := GetMinMax(ipList)
		ipRange := ipMax - ipMin
		if ipRange >= uint32(200) {
			ipNet := Uitoin(ipfw3, UINT24_MAX)
			attackIPNets = append(attackIPNets, ipNet)
		} else {
			if len(ipList) >= ipSegCoverThreshold {
				ipNet := Uitoin(ipfw3, UINT24_MAX)
				attackIPNets = append(attackIPNets, ipNet)
			} else {
				ipNets := IPRangeToCIDR(ipMin, ipMax)
				attackIPNets = append(attackIPNets, ipNets...)
			}
		}

	}
	return attackIPNets
}

// ip merge 3
func IPSuspectFW2(ipCountDF map[uint32]int, ipNumThreshold int, ipSegCoverThreshold int) []*net.IPNet {
	//get ipfw3df
	//get ip
	ipCountFw2DF := make(map[uint32]int)
	ipSusIPFw2P3DF := make(map[uint32][]uint32)
	ipSusIPFw2DF := make(map[uint32][]uint32)
	for ip, count := range ipCountDF {
		ipFw2 := ip & (0xFFFF0000)
		ipP3 := ip & (0x0000FF00)
		ipCountFw2DF[ipFw2] += count
		//get ipfw2-ipp3 map
		ipp3s, ok := ipSusIPFw2P3DF[ipFw2]
		if !ok {
			ipp3s = make([]uint32, 0)
		}
		ipp3s = append(ipp3s, ipP3)
		ipSusIPFw2P3DF[ipFw2] = ipp3s
		//get ipfw3-ipp4 map
		ips, ok := ipSusIPFw2DF[ipFw2]
		if !ok {
			ips = make([]uint32, 0)
		}
		ips = append(ips, ip)
		ipSusIPFw2DF[ipFw2] = ips
	}
	//get ipfw3 ips
	IPFW2 := make([]uint32, 0)
	if len(ipCountFw2DF) >= ipNumThreshold {
		for ipfw2, count := range ipCountFw2DF {
			if count > ipNumThreshold/2 {
				IPFW2 = append(IPFW2, ipfw2)
			}
		}
	} else {
		for ipfw2, count := range ipCountFw2DF {
			if count > ipNumThreshold {
				IPFW2 = append(IPFW2, ipfw2)
			}
		}
	}
	//merge
	attackIPNets := make([]*net.IPNet, 0)
	for _, ipfw2 := range IPFW2 {
		//delete ips from map
		for _, ip := range ipSusIPFw2DF[ipfw2] {
			delete(ipCountDF, ip)
		}
		//merge ip in different condition
		ipList := ipSusIPFw2DF[ipfw2]
		ipp3List := ipSusIPFw2P3DF[ipfw2]
		ipp3Min, ipp3Max := GetMinMax(ipp3List)
		if len(ipList) >= ipSegCoverThreshold && len(ipp3List) >= ipSegCoverThreshold {
			ipNet := Uitoin(ipfw2, UINT16_MAX)
			attackIPNets = append(attackIPNets, ipNet)
		} else {
			ipMin, ipMax := (ipp3Min + ipfw2), (ipp3Max + ipfw2)
			ipNets := IPRangeToCIDR(ipMin, ipMax)
			attackIPNets = append(attackIPNets, ipNets...)
		}
	}
	return attackIPNets
}

// ip random
func IPSuspectRand(normal_ips map[uint32]struct{}, ipCountDF map[uint32]int, ipRandCountThreshold int, ipRandNumThreshold int) bool {
	//get all normal ips
	for ip, count := range ipCountDF {
		if count > ipRandCountThreshold {
			normal_ips[ip] = struct{}{}
		}
	}
	//delete all normal ips from ipCountDF
	for ip := range normal_ips {
		_, ok := ipCountDF[ip]
		if ok {
			delete(ipCountDF, ip)
		}
	}
	//get ipUnknownRandCountSum
	var ipUnknownRandCountSum int
	for _, count := range ipCountDF {
		ipUnknownRandCountSum += count
	}
	return ipUnknownRandCountSum >= ipRandNumThreshold
}

type NumberStatsType struct {
	Stat        string
	KeyCountMap map[uint32]int
}

func NewNumberStatsType() *NumberStatsType {
	return &NumberStatsType{
		KeyCountMap: make(map[uint32]int),
	}
}

type ProtoStatsType struct {
	ProtoStatMap  map[uint32]string
	ProtoCountMap map[uint32]int
}

func NewProtoStatsType() *ProtoStatsType {
	return &ProtoStatsType{
		ProtoCountMap: make(map[uint32]int),
		ProtoStatMap:  make(map[uint32]string),
	}
}

type SIPBaseMergedStat struct {
	HostNicSign string
	SIP_RAND    bool
	SIP         string
	IPIdxs      []int
	DIP         *NumberStatsType
	SPort       *NumberStatsType
	DPort       *NumberStatsType
	Length      *NumberStatsType
	Proto       *ProtoStatsType
}

func NewSIPBaseMergeStat() *SIPBaseMergedStat {
	return &SIPBaseMergedStat{
		IPIdxs: make([]int, 0),
		DIP:    NewNumberStatsType(),
		SPort:  NewNumberStatsType(),
		DPort:  NewNumberStatsType(),
		Length: NewNumberStatsType(),
		Proto:  NewProtoStatsType(),
	}
}

type FiveTuple struct {
	SIP_RAND    bool
	HostNicSign string
	SIP         string
	DIP         string
	SPort       string
	DPort       string
	Proto       string
	Length      map[int]struct{}
}

func NewFiveTuple() *FiveTuple {
	return &FiveTuple{
		Length: make(map[int]struct{}),
	}
}

func SIPBaseMerge(hostNicSign string, pinfos []map[string]uint32, attakIPNets []*net.IPNet, is_rand bool, ipCountDF4 map[uint32]int) map[string]*SIPBaseMergedStat {
	//merged results
	mergedStats := make(map[string]*SIPBaseMergedStat)
	//merge other infos based on sip
	for ip_idx, info := range pinfos {
		ip := info["SIP"]
		dip := info["DIP"]
		sport := info["SPort"]
		dport := info["DPort"]
		length := info["Length"]
		proto := info["Proto"]
		for _, attakIPNet := range attakIPNets {
			if attakIPNet.Contains(net.IP(Uiton(ip))) {
				sipnet := attakIPNet.String()
				mergedStat, ok := mergedStats[sipnet]
				if !ok {
					mergedStat = NewSIPBaseMergeStat()
				}
				mergedStat.HostNicSign = hostNicSign
				mergedStat.SIP = sipnet
				mergedStat.IPIdxs = append(mergedStat.IPIdxs, ip_idx)
				mergedStat.DIP.KeyCountMap[dip]++
				mergedStat.SPort.KeyCountMap[sport]++
				mergedStat.DPort.KeyCountMap[dport]++
				mergedStat.Length.KeyCountMap[length]++
				mergedStat.Proto.ProtoCountMap[proto]++
				mergedStats[sipnet] = mergedStat
				break
			}
		}
		//random station
		if is_rand {
			_, ok := ipCountDF4[ip]
			if ok {
				mergedStat, ok := mergedStats["random"]
				if !ok {
					mergedStat = NewSIPBaseMergeStat()
				}
				mergedStat.HostNicSign = hostNicSign
				mergedStat.SIP = "random"
				mergedStat.IPIdxs = append(mergedStat.IPIdxs, ip_idx)
				mergedStat.DIP.KeyCountMap[dip]++
				mergedStat.SPort.KeyCountMap[sport]++
				mergedStat.DPort.KeyCountMap[dport]++
				mergedStat.Length.KeyCountMap[length]++
				mergedStat.Proto.ProtoCountMap[proto]++
				mergedStats["random"] = mergedStat
			}
		}
	}
	return mergedStats
}

type DMSRule struct {
	HostNicSign    string
	SRCIP          string
	DSTIP          string
	SPort          string //
	DPort          string //
	Length         string
	Proto          int    //
	Action         string //default drop
	LimitMode      string //"bps" or "pps"
	LimnitMaxValue uint64
}

func GenDMSRules(mergedStats *SIPBaseMergedStat, netdev []float64) (rules []DMSRule) {
	//action judge
	action := "drop"
	var limitMode string
	var limitValue uint64
	if !(mergedStats.SPort.Stat == "centralize" || mergedStats.DPort.Stat == "centralize" || !mergedStats.SIP_RAND) {
		action = "limit"
		//caculate limit value
		limitMode, limitValue = CaculateLimitValue(netdev)
	}
	//sport stats judge
	var sports []int
	if mergedStats.SPort.Stat == "centralize" {
		i := 0
		sports = make([]int, len(mergedStats.SPort.KeyCountMap))
		for port := range mergedStats.SPort.KeyCountMap {
			sports[i] = int(port)
			i++
		}
	} else {
		sports = []int{-1}
	}
	//dport stats judge
	var dports []int
	if mergedStats.SPort.Stat == "centralize" {
		i := 0
		dports = make([]int, len(mergedStats.DPort.KeyCountMap))
		for port := range mergedStats.DPort.KeyCountMap {
			dports[i] = int(port)
			i++
		}
	} else {
		dports = []int{-1}
	}
	//gen rules
	rule_num := len(mergedStats.DIP.Stat) * len(sports) * len(dports) * len(mergedStats.Proto.ProtoCountMap)
	rules = make([]DMSRule, rule_num)
	rule_id := 0
	for proto := range mergedStats.Proto.ProtoCountMap {
		for dip := range mergedStats.DIP.KeyCountMap {
			for _, sport := range sports {
				for _, dport := range dports {
					rule := DMSRule{}
					rule.Action = action
					rule.LimitMode = limitMode
					rule.LimnitMaxValue = limitValue
					rule.HostNicSign = mergedStats.HostNicSign
					rule.SRCIP = mergedStats.SIP
					rule.DSTIP = Uitoa(dip)
					rule.SPort = strconv.Itoa(sport)
					rule.DPort = strconv.Itoa(dport)
					rule.Proto = int(proto)
					rule_id++
				}
			}
		}
	}
	return rules
}

func GenDMSRulesDedup(fiveTupleValue map[string]string, netdev []float64) DMSRule {
	//hostnic、sip、dip、proto
	hostNic := fiveTupleValue["HostNicSign"]
	sip := fiveTupleValue["SIP"]
	dip := fiveTupleValue["DIP"]
	proto, _ := strconv.Atoi(fiveTupleValue["Proto"])
	var sport_central, dport_central bool
	//sport stats judge
	_, err := strconv.Atoi(fiveTupleValue["SPort"])
	if err == nil {
		sport_central = true
	}
	//dport stats judge
	_, err = strconv.Atoi(fiveTupleValue["SPort"])
	if err == nil {
		dport_central = true
	}
	//action judge
	action := "drop"
	var limitMode string
	var limitValue uint64
	//get status
	if !(fiveTupleValue["SIP"] != "random" || sport_central || dport_central) {
		action = "limit"
		//caculate limit value
		limitMode, limitValue = CaculateLimitValue(netdev)
	}
	rule := DMSRule{
		SRCIP:          sip,
		DSTIP:          dip,
		SPort:          fiveTupleValue["SPort"],
		DPort:          fiveTupleValue["DPort"],
		Length:         fiveTupleValue["Length"],
		Proto:          proto,
		HostNicSign:    hostNic,
		Action:         action,
		LimitMode:      limitMode,
		LimnitMaxValue: limitValue,
	}
	return rule
}

func CaculateLimitValue(netdev []float64) (limitMode string, limitValue uint64) {
	//bps value,bps score,pps value,pps score
	if netdev[1]-netdev[3] > 1e-6 {
		limitMode = "bps"
		limitValue = uint64((netdev[0] / 2))
	} else {
		limitMode = "pps"
		limitValue = uint64((netdev[2] / 2))
	}
	return limitMode, limitValue
}
