package generator

import (
	"fmt"
	"math"

	"github.com/amianetworks/dni/sdk/task/dms/assessor"
	"github.com/amianetworks/dni/sdk/utils"
	"github.com/mitchellh/mapstructure"
	"github.com/shirou/gopsutil/cpu"
)

type AssessorDataGenerator struct {
	Name            string
	devs            []string
	cpuAll, cpuSoft float64
	bwStat          map[string]map[string]uint64
}

type AssessDataOptions struct {
	Devs []string `mapstructure:"devs"`
}

func NewAssessorDataGenerator(name string, options interface{}) (*AssessorDataGenerator, error) {
	var opts AssessDataOptions
	err := mapstructure.Decode(options, &opts)
	if err != nil {
		return nil, fmt.Errorf("%s options decode error:%v", name, err)
	}
	ag := &AssessorDataGenerator{
		devs: opts.Devs,
	}
	return ag, nil
}

func (g *AssessorDataGenerator) PrepareForRun() error {
	//init cpu
	t, err := cpu.Times(false)
	if err != nil {
		return fmt.Errorf("%s get cpu statistics error:%v", g.Name, err)
	}
	g.cpuAll, g.cpuSoft = GetSoftirq(t[0])
	//init bw
	g.bwStat = make(map[string]map[string]uint64)
	for _, dev := range g.devs {
		bw, err := GetNetifStat(dev)
		if err != nil {
			return fmt.Errorf("%s get bandwidth statistics error:%v", g.Name, err)
		}
		g.bwStat[dev] = bw
	}
	return nil
}

func (g *AssessorDataGenerator) GetAssessorData() (assessor.AssessorData, error) {
	cpu, err := g.getCPU()
	if err != nil {
		return assessor.AssessorData{}, err
	}
	bw, err := g.getBW()
	if err != nil {
		return assessor.AssessorData{}, err
	}
	tcpconn, err := GetTCPConnBriefProc()
	if err != nil {
		return assessor.AssessorData{}, err
	}
	snmp, err := GetSNMP()
	if err != nil {
		return assessor.AssessorData{}, err
	}
	res := assessor.AssessorData{
		BW:      make(map[string]map[string]uint64),
		TCPConn: make(map[string]uint64),
		SNMP:    make(map[string]uint64),
	}
	res.CPU = cpu
	res.BW = bw
	res.SNMP = snmp
	res.TCPConn = tcpconn
	return res, nil
}

func (ag *AssessorDataGenerator) getCPU() (float64, error) {
	t, err := cpu.Times(false)
	if err != nil {
		return 0.0, err
	}
	lastAll, lastSoft := ag.cpuAll, ag.cpuSoft
	ag.cpuAll, ag.cpuSoft = GetSoftirq(t[0])
	return math.Min(100, math.Max(0, (ag.cpuSoft-lastSoft)/(ag.cpuAll-lastAll)*100)), nil
}

func (ag *AssessorDataGenerator) getBW() (map[string]map[string]uint64, error) {
	allStatDiff := make(map[string]map[string]uint64)
	for _, dev := range ag.devs {
		lastStat := ag.bwStat[dev]
		currStat, err := GetNetifStat(dev)
		if err != nil {
			return nil, err
		}
		statDiff := make(map[string]uint64)
		for k, v := range currStat {
			if v != 0 {
				if v >= currStat[k] {
					statDiff[k] = v - lastStat[k]
				} else { // cross the upper bound
					statDiff[k] = utils.UINT64_MAX - (lastStat[k] - v)
				}
			}
		}
		allStatDiff[dev] = statDiff
		ag.bwStat[dev] = currStat
	}
	return allStatDiff, nil
}
