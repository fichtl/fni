package analyzer

import (
	"net"

	"github.com/amianetworks/dni/sdk/task/dms/common"
)

func mergeIps(in []common.IPCount) []string {
	new := make([]common.IRange, len(in))
	for i, ip := range in {
		new[i] = common.IPWrapper{IP: net.ParseIP(ip.IP)}
	}
	var out []string
	for _, irange := range common.MergeIRanges(new) {
		for _, n := range irange.ToIPNets() {
			out = append(out, n.String())
		}
	}
	return out
}

func estimatedSpeed(val int, ratio float64) int {
	return int(float64(val) / ratio)
}

func estimatedThresh(val int, ratio float64) int {
	return int(float64(val) * ratio)
}
