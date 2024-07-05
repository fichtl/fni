package analyzer

import (
	"math"
	"sort"

	"github.com/amianetworks/dni/sdk/task/dms/common"
	"github.com/amianetworks/dni/sdk/task/dms/config"
)

// TODO: RuleCap to limit rule generation.
const (
	RuleCap = 8192
)

// isLinear checks if the sorted sequence is linear
func isLinear(arr []uint32) bool {
	diff := make(map[uint32]int)
	n := len(arr) - 1
	for i := 0; i < n; i++ {
		ni := diff[arr[i+1]-arr[i]]
		ni++
		if float32(ni)/float32(n) > 0.8 {
			return false
		}
		diff[arr[i+1]-arr[i]] = ni
	}
	return true
}

func isRandSrc0(ipStat []common.IPCount, th config.Capability) bool { return len(ipStat) >= th.N }

func isRandSrc1(ipStat []common.IPCount, th config.Capability) (bool, []uint32) {
	var rev = make(map[uint64][]uint32)
	for _, s := range ipStat {
		key := s.Count
		ip := common.Atoui(s.IP)
		l, ok := rev[key]
		if !ok {
			rev[key] = make([]uint32, 1)
			rev[key][0] = ip
		} else {
			rev[key] = append(l, ip)
		}
	}
	for _, l := range rev {
		if len(l) < th.N {
			continue
		}
		sort.Sort(common.IPSlice(l))
		// if source IP addresses are linear generated, it can be easily predicted
		if isLinear(l) {
			return false, l
		} else {
			return true, nil
		}
	}
	return false, nil
}

func isRandSrc2(ipStat []common.IPCount, th config.Capability) (bool, []uint32) {
	n := len(ipStat)
	i := sort.Search(n, func(k int) bool { return ipStat[k].Count <= 100 })
	if n-i < th.N {
		return false, nil
	}
	l := make([]uint32, n-i)
	for j := 0; j < n-i; j++ {
		l[j] = common.Atoui(ipStat[i+j].IP)
	}
	sort.Sort(common.IPSlice(l))
	if isLinear(l) {
		return false, l
	} else {
		return true, nil
	}
}

func isRandSrc3(atk, susp []common.IPCount, th config.Capability) (bool, []string) {
	spoofed := mergeIps(atk)
	cap := th.N
	randSrc := len(atk)+len(susp) >= cap &&
		(len(atk) <= th.Strict*len(spoofed) || len(spoofed) > cap)
	return randSrc, spoofed
}

// isRandSrc check if source IP addresses are random generated.
//
// There are several solutions for this problem:
//
// isRandSrc0: Check if the number of spoofed source IP addresses is larger than the
// thresholds. If so, we believe its a random-sourced DDoS attack.
//
// isRandSrc1: Assumes that attackers only spoof IP addresses, so request count of
// spoofed addresses are similar. Therefore, we put IP addresses with the same request
// count to see if these addresses are random generated or linear distributed. Still,
// it's not a solid solution.
//
// isRandSrc2: Aggregates all sources that has count that is less than 100 and checks if
// it's linear. `isRandSrc1' has finer granularity but higher time complexity.
func isRandSrc(ipStat []common.IPCount, th config.Capability) (bool, []uint32) {
	return len(ipStat) >= th.N, nil
	// return isRandSrc1(ipStat)
	// return isRandSrc2(ipStat)
}

func isRandPort0(portStat []common.PortCount, th config.Capability) bool {
	return len(portStat) >= th.N
}

// isRandPort check if port statistic is random generated.
//
// isRandPort0: Check if the number of spoofed ports is larger than the thresholds. If
// so, we believe its a random-sourced DDoS attack.
func isRandPort(portStat []common.PortCount, th config.Capability) bool {
	return isRandPort0(portStat, th)
}

// classifySrcIPs splits `STATS' by `CT'.
//
// If dms is running on host with vm or containers, regard all ports as spoofed.
// Otherwise, put not established IPs into `spoofed'.
//
// - spoofed: not established and traffic rate is large
//
// - affected: established but traffic rate is still high, need further analysis
//
// - active: established and traffic rate is tolerable
//
// - susp: not established while traffic rate is small
//
// Note: comparing to `filterSpoofedPorts', we don't provide array of active/established
// IPStat since we don't want to limit traffic rate for already established IP addresses.
// This is dangerous if established peers are also bots, we handle this situation at
// connection-based analysis or inhibit it from source.
func classifySrcIPs(stats []common.IPCount, est map[string]struct{}, th uint64) (newhigh, esthigh, newlow, estlow []common.IPCount) {
	// spoofed := make(map[string]struct{})
	//结合连接表和阈值给源ip分类为：未建立连接的高频IP，建立连接的高频IP，未建立连接的低频IP，建立连接的低频IP
	for _, stat := range stats {
		if stat.Count > th {
			if _, ok := est[stat.IP]; !ok {
				newhigh = append(newhigh, stat)
			} else {
				esthigh = append(esthigh, stat)
			}
		} else {
			if _, ok := est[stat.IP]; !ok {
				newlow = append(newlow, stat)
			} else {
				estlow = append(estlow, stat)
			}
		}
	}

	return newhigh, esthigh, newlow, estlow
}

// filterSpoofedSrcPorts splits `portStat' by `estPorts'.
//
// If dms is running on host with nfv environment, regard all traffic to virt endpoints
// as benign. There are three kinds of traffic features that indicates whether the flow
// is directing to nfv or not: 1. packet with dst IP directly to virt endpoint; 2. NATed
// traffics; 3. port forwarded;
//
// Otherwise, put active (currently established) ports into `active' and others into
// `spoofed'.
func filterSpoofedSrcPorts(stats []common.PortCount, estPorts map[common.PORT]struct{}, tol uint64) ([]common.PortCount, []common.PortCount) {
	spoofed := make([]common.PortCount, 0)
	active := make([]common.PortCount, 0)

	for _, stat := range stats {
		// Avoids adding benign sources that accidentially initiate connections at
		// sampling phase. FIXME: Currently, if traffic rate on this port is small, we
		// ignore these ports.
		if stat.Count < tol {
			break
		}
		// Host shouldn't filter port provided by VM or Container
		if _, ok := estPorts[stat.Port]; !ok {
			spoofed = append(spoofed, stat)
		} else {
			active = append(active, stat)
		}
	}

	return spoofed, active
}

// filterSpoofedDstPorts splits `portStat' by `lisPorts' and `estPorts'.
//
// If dms is running on host with vm or containers, regard all ports as active. (I don't
// know why. I just keep this logic)
//
// If a port is not listening, but detected on dataplane, we believe these ports are
// spoofed and should be dropped with no tolerance.
//
// If a port is listening, check if it's under attack (no established connections) is
// listening,
//
// FIXME
func filterSpoofedDstPorts(stats []common.PortCount, lisPorts map[common.PORT]struct{}, estPorts map[common.PORT]map[string]struct{}) (spoofed, affected, active []common.PortCount) {
	for _, stat := range stats {
		if _, ok := lisPorts[stat.Port]; ok {
			if _, est := estPorts[stat.Port]; est {
				active = append(active, stat) // benign ports
			} else {
				affected = append(affected, stat) // affected ports
			}
		} else {
			spoofed = append(spoofed, stat) // spoofed ports
		}
	}
	return spoofed, affected, active
}

// randomRequestsByEntropy checks the randomness of distribution of requests from
// distinct sources by calculating the entropy of its histogram. If its information gain
// against uniform distribution is larger than a threshold, we believe it's random.
func randomRequestsByEntropy(hist []common.PeerStat) bool {
	var (
		n          = float64(len(hist))
		total      uint64
		entropy    float64
		maxEntropy = -math.Log2(1.0 / n)
	)
	for _, s := range hist {
		total += s.GetCount()
	}
	for _, s := range hist {
		f := float64(s.GetCount()) / float64(total)
		if f == 0 {
			continue
		}
		entropy += -f * math.Log2(f)
	}
	return (maxEntropy-entropy)/maxEntropy > 0.02
}
