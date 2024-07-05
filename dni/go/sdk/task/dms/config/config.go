package config

import "math"

type CPUUtilThreshold struct {
	Low      float64 `mapstructure:"low"`
	Moderate float64 `mapstructure:"okay"`
	High     float64 `mapstructure:"high"`
}

type BwUtilThreshold struct {
	BPS int `mapstructure:"bps"`
	PPS int `mapstructure:"kpps"`
}

type ProtoThreshold struct {
	Threshold      int            `mapstructure:"threshold"`
	ProtoThreshold map[string]int `mapstructure:"protothreshold"`
	Tolerance      float64        `mapstructure:"tolerance"`
}

func (pps *ProtoThreshold) GetPPSThreshold(proto string, withTol bool) int {
	th, ok := pps.ProtoThreshold[proto]
	if !ok {
		th = pps.Threshold
	}
	if withTol {
		th = int(math.Floor(float64(th) * (1 + pps.Tolerance)))
	}
	return th
}

type TcpConnUtilThreshold struct {
	SemiConn        int `mapstructure:"tcpsemiconn"`
	FullConn        int `mapstructure:"tcpfullconn"`
	PerPortFullConn int `mapstructure:"pertcpfullconn"` // Set it to -1 to ignore this pattern.
}

type Capability struct {
	N      int `mapstructure:"cap"`
	Strict int `mapstructure:"strictness"`
}
