package analyzer

import (
	"github.com/amianetworks/dni/sdk/task/dms/common"
	"github.com/amianetworks/dni/sdk/task/dms/config"
)

func FloodAttackDetection(protodiag *ProtoDiag, th config.ProtoThreshold, cap config.Capability, ct *common.CtInfo) []*Respond {
	resps := make([]*Respond, 0)
	if protodiag == nil {
		return resps
	}
	for dev, stat := range protodiag.Stat {
		//ICMP Attack
		thicmp := th.GetPPSThreshold(common.PROTO_STR_ICMP, true)
		for k, v := range stat.ICMP.Type {
			if estimatedSpeed(v, stat.Ratio) < thicmp {
				continue
			}
			stat.ICMP.SIP = stat.raw.icmpsrc.SortByCount(-1)
			resp := newRespond()
			resp.Dev = dev
			if k == common.ICMPv4TypeDestinationUnreachable { // Regard as UDP reflection attack
				resp.Type[ATTACK_REFLECT_UDP] = 1
			} else { // Regard as other ICMP flood attacks
				resp.Type[ATTACK_ICMP] = 1
			}
			diagICMPAttack(&resp.AttackInfo, &stat.ICMP, ct.Est, cap)
			resps = append(resps, resp)
			break // this logic need only run once
		}

		//TCP Attack
		thtcp := th.GetPPSThreshold(common.PROTO_STR_TCP, true)
		if estimatedSpeed(stat.TCP.Total, stat.Ratio) < thtcp {
			goto endtcp
		}
		for flag, count := range stat.TCP.Flag {
			//different tcp flood speed
			if estimatedSpeed(count, stat.Ratio) < thtcp {
				continue
			}
			resp := newRespond()
			switch common.TCPFlag(flag) {
			// Packet with empty TCP flag is malformed
			case 0:
				resp.Pattern = common.NewRuleSet(genTCPFlagRule("", common.ACTION_DROP, 0))
				resp.Type[ATTACK_TCP] = 1

			case common.TCP_FLAG_SYN, common.TCP_FLAG_SYNPSH:
				data := calcTransStat(stat.raw.tcp.flag[flag], ct)
				diagTCPSynFlood(&resp.AttackInfo, data, flag, th, cap, protodiag.Tcp, ct.Est)

			case common.TCP_FLAG_ACK, common.TCP_FLAG_ACKPSH:
				data := calcTransStat(stat.raw.tcp.flag[flag], ct)
				thtcp_ := uint64(estimatedThresh(thtcp, stat.Ratio))
				diagTCPFlood(&resp.AttackInfo, data, flag, thtcp_, th, cap, protodiag.Tcp, ct.Est)

			case common.TCP_FLAG_FIN, common.TCP_FLAG_RST:
				data := calcTransStat(stat.raw.tcp.flag[flag], ct)
				thtcp_ := uint64(estimatedThresh(thtcp, stat.Ratio))
				diagTCPFlood(&resp.AttackInfo, data, flag, thtcp_, th, cap, protodiag.Tcp, ct.Est)

			case common.TCP_FLAG_SYNACK, common.TCP_FLAG_FINACK, common.TCP_FLAG_RSTACK:
				data := calcTransStat(stat.raw.tcp.flag[flag], ct)
				thtcp_ := uint64(estimatedThresh(thtcp, stat.Ratio))
				diagTCPFlood(&resp.AttackInfo, data, flag, thtcp_, th, cap, protodiag.Tcp, ct.Est)

			default:
				continue
			}
			resp.Type[flagmap[common.TCPFlag(flag)]] = 1
			resp.Type[ATTACK_TCP] = 1
			resp.Dev = dev
			resps = append(resps, resp)
		}

	endtcp:
		//UDP Flood
		thudp := th.GetPPSThreshold(common.PROTO_STR_UDP, true)
		if estimatedSpeed(stat.UDP.Total, stat.Ratio) < thtcp {
			continue
		}
		resp := newRespond()
		data := calcTransStat(stat.raw.udp.metas, ct)
		diagUDPFlood(&resp.AttackInfo, data, uint64(estimatedThresh(thudp, stat.Ratio)), th, cap, protodiag.Udp, ct.Est)
		resp.Type[ATTACK_UDP] = 1
		resp.Dev = dev
		resps = append(resps, resp)
	}
	return resps
}

func diagICMPAttack(a *AttackInfo, stat *ICMPStat, normalIPs map[string]struct{}, cap config.Capability) {
	rs := common.NewRuleSet()

	spoofed, _, _, _ := classifySrcIPs(stat.SIP, normalIPs, 0)
	if isRandSrc0(spoofed, cap) {
		a.isDistributed = true
	} else {
		r := common.Rule{}
		for _, s := range spoofed {
			r.Type = "l3"
			r.SrcIP = s.IP
			r.IPProto = common.PROTO_STR_ICMP
			rs.Add(r)
		}
	}
	a.Pattern = rs
}

func diagTCPFlood(a *AttackInfo, stat *transStat, flag uint8, thtcp uint64, th config.ProtoThreshold, cap config.Capability, tcp *common.PortInfo, normalIPs map[string]struct{}) {
	rs := common.NewRuleSet()
	noop := common.NewRuleSet()

	thddos := cap.N
	attackers, _, tolerable, _ := classifySrcIPs(stat.SIP, normalIPs, thtcp)
	if rand, spoofed := isRandSrc3(attackers, tolerable, cap); !rand && len(stat.DstPort) <= thddos {
		for _, s := range spoofed {
			rs.Add(genL3Rule(s, 0, 0))
		}
	} else if len(stat.DstPort) <= thddos {
		a.randSrc = true
		for p := range stat.DstPort {
			if _, ok := tcp.Listen[p]; !ok {
				rs.Add(genPortRule(common.PROTO_STR_TCP, p, false, 0, 0))
			} else {
				noop.Add(genPortRule(common.PROTO_STR_TCP, p, false, 0, 0))
			}
		}
	} else if !rand {
		a.randDstPort = true
		for _, s := range spoofed {
			rs.Add(genL3Rule(s, 0, 0))
		}
	} else {
		//random sip && random dport
		a.randSrc = true
		a.randDstPort = true
		noop.Add(genTCPFlagRule(common.TCPFlag(flag).String(), common.ACTION_LIMIT_PPS, uint64(th.GetPPSThreshold(common.PROTO_STR_TCP, false))))
	}

	a.Pattern = rs
	a.RiskyPattern = noop
}

func diagTCPSynFlood(a *AttackInfo, stat *transStat, flag uint8, th config.ProtoThreshold, cap config.Capability, tcp *common.PortInfo, normalIPs map[string]struct{}) {
	rs := common.NewRuleSet()
	noop := common.NewRuleSet()
	thddos := cap.N
	attackers, _, susp, _ := classifySrcIPs(stat.SIP, normalIPs, 1)
	// TODO: merge spoofed and abnormal to handle high speed syn from established host
	if rand, spoofed := isRandSrc3(attackers, susp, cap); !rand {
		for _, s := range spoofed {
			rs.Add(genL3Rule(s, 0, 0))
		}
	} else if len(stat.DstPort) <= thddos {
		a.randSrc = true
		//random src && fixed dport
		for p := range stat.DstPort {
			if _, ok := tcp.Listen[p]; !ok {
				rs.Add(genPortRule(common.PROTO_STR_TCP, p, false, 0, 0))
			} else {
				noop.Add(genPortRule(common.PROTO_STR_TCP, p, false, 0, 0))
			}
		}
	} else {
		a.randSrc = true
		a.randDstPort = true
	}

	a.Pattern = rs
	a.RiskyPattern = noop
}

func diagTCPAckFlood(a *AttackInfo, stat *transStat, flag uint8, thtcp uint64, th config.ProtoThreshold, cap config.Capability, tcp *common.PortInfo, normalIPs map[string]struct{}) {
	rs := common.NewRuleSet()
	noop := common.NewRuleSet()
	thddos := cap.N
	attackers, _, tolerable, _ := classifySrcIPs(stat.SIP, normalIPs, thtcp)
	if rand, spoofed := isRandSrc3(attackers, tolerable, cap); !rand {
		for _, s := range spoofed {
			rs.Add(genL3Rule(s, 0, 0))
		}
	} else if len(stat.DstPort) <= thddos {
		a.randSrc = true
		for p := range stat.DstPort {
			if _, ok := tcp.Listen[p]; !ok {
				rs.Add(genPortRule(common.PROTO_STR_TCP, p, false, 0, 0))
			} else {
				noop.Add(genPortRule(common.PROTO_STR_TCP, p, false, 0, 0))
			}
		}
	} else {
		a.randSrc = true
		a.randDstPort = true
		//对flag限速
		noop.Add(genTCPFlagRule("ACK", common.ACTION_LIMIT_PPS, uint64(th.GetPPSThreshold("tcp", false))))
	}

	a.Pattern = rs
	a.RiskyPattern = noop
}

func diagFinRstFlood(a *AttackInfo, stat *transStat, flag uint8, thtcp uint64, th config.ProtoThreshold, cap config.Capability, tcp *common.PortInfo, normalIPs map[string]struct{}) {
	rs := common.NewRuleSet()
	noop := common.NewRuleSet()

	thddos := cap.N

	attackers, _, tolerable, _ := classifySrcIPs(stat.SIP, normalIPs, thtcp)
	if rand, spoofed := isRandSrc3(attackers, tolerable, cap); !rand && len(stat.DstPort) <= thddos {
		for _, s := range spoofed {
			rs.Add(genL3Rule(s, 0, 0))
		}
	} else if len(stat.DstPort) <= thddos {
		a.randSrc = true
		for p := range stat.DstPort {
			// if _, ok := ct.EstTcp[p]; ok {
			// 	continue
			// }
			if _, ok := tcp.Listen[p]; !ok {
				rs.Add(genPortRule(common.PROTO_STR_TCP, p, false, 0, 0))
			} else {
				noop.Add(genPortRule(common.PROTO_STR_TCP, p, false, 0, 0))
			}
		}
	} else if !rand {
		a.randDstPort = true
		for _, s := range spoofed {
			rs.Add(genL3Rule(s, 0, 0))
		}
	} else {
		a.randSrc = true
		a.randDstPort = true
	}

	a.Pattern = rs
	a.RiskyPattern = noop
}

// UDP diagnostics
func diagUDPFlood(a *AttackInfo, stat *transStat, thudp uint64, th config.ProtoThreshold, cap config.Capability, udp *common.PortInfo, normalIPs map[string]struct{}) {
	rs := common.NewRuleSet()
	noop := common.NewRuleSet()
	thddos := cap.N
	attackers, _, tolerable, _ := classifySrcIPs(stat.SIP, map[string]struct{}{}, thudp)
	if rand, spoofed := isRandSrc3(attackers, tolerable, cap); !rand {
		for _, s := range spoofed {
			rs.Add(genL3Rule(s, 0, 0))
		}
	} else if len(stat.DstPort) <= thddos {
		a.randSrc = true
		for p := range stat.DstPort {
			// if _, ok := ct.EstUdp[p]; ok {
			// 	continue
			// }
			if _, ok := udp.Listen[p]; !ok {
				rs.Add(genPortRule(common.PROTO_STR_UDP, p, false, 0, 0))
			} else {
				noop.Add(genPortRule(common.PROTO_STR_UDP, p, false, 0, 0))
			}
		}
	} else {
		a.randSrc = true
		a.randDstPort = true
	}
	a.Pattern = rs
	a.RiskyPattern = noop
}

// TCP diagnostics
type transStat struct {
	// DDoS
	SIP  []common.IPCount // Deprecated
	Peer []common.IPPStat

	SrcPort common.PortCountMap // Reflection DDoS
	DstPort common.PortCountMap
	PerPort map[common.PORT][]common.IPCount // Per-port ip statistics
	PP      map[common.PORT][]common.IPPStat // TODO: per-port ipp statistics

	// TODO: super spreader
	DIP  []common.IPCount
	DIPP []common.IPPStat
}

func newTransStat() *transStat {
	return &transStat{
		SIP:     make([]common.IPCount, 0),
		Peer:    make([]common.IPPStat, 0),
		PerPort: make(map[common.PORT][]common.IPCount),
		PP:      make(map[common.PORT][]common.IPPStat),
		SrcPort: make(common.PortCountMap),
		DstPort: make(common.PortCountMap),
		DIP:     make([]common.IPCount, 0),
		DIPP:    make([]common.IPPStat, 0),
	}
}

func calcTransStat(data []*pktMeta, ct *common.CtInfo) *transStat {
	ret := newTransStat()
	src := make(common.IPCountMap)
	dst := make(common.IPCountMap)
	sport := make(common.PortCountMap)
	dport := make(common.PortCountMap)
	// ipp := make(map[common.PORT]common.IPCountMap)
	// pp := make(map[common.PORT]common.IPPStatMap)
	for _, m := range data {
		// Filtering out traffics with source IPs that are forwarded to NFV containers.
		// Server-mode DMS only protects traffics that are sent to host's CPU.
		// if common.EnvRunIn == common.PLATF_WITH_VIRT {
		// 	if _, ok := ct.Rtd[m.Rip]; ok {
		// 		continue
		// 	}
		// 	if _, ok := ct.Nat[m.Rip]; ok {
		// 		continue
		// 	}
		// }
		src[m.Rip] += 1
		dst[m.Lip] += 1
		sport[m.Rport] += 1
		dport[m.Lport] += 1
	}
	ret.SIP = src.SortByCount(-1)
	ret.DIP = dst.SortByCount(-1)
	ret.SrcPort = sport
	ret.DstPort = dport
	return ret
}
