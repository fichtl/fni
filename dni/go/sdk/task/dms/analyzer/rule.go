// TODO: move to common
package analyzer

import (
	"github.com/amianetworks/dni/sdk/task/dms/common"
)

func genL3Rule(ip string, ltype int8, lrate uint64) (r common.Rule) {
	r.Type = "l3"
	r.SrcIP = ip
	if lrate == 0 {
		return
	}
	r.Drop = ltype
	r.Rate = lrate
	return
}

func genTCPFlagRule(flag string, ltype int8, lrate uint64) (r common.Rule) {
	r.Type = "l4"
	r.IPProto = common.PROTO_STR_TCP
	r.TCPFlags = flag
	if lrate == 0 {
		return
	}
	r.Drop = ltype
	r.Rate = lrate
	return
}

func genProtoRule(proto string, ltype int8, lrate uint64) (r common.Rule) {
	r.Type = "l4"
	r.IPProto = proto
	if lrate == 0 {
		return
	}
	r.Drop = ltype
	r.Rate = lrate
	return
}

func genPortRule(proto string, port common.PORT, isSrcPort bool, ltype int8, lrate uint64) (r common.Rule) {
	r.Type = "l4"
	r.IPProto = proto
	if proto == common.PROTO_STR_ICMP {
		goto action
	}
	if isSrcPort {
		r.SrcPort = port.String()
	} else {
		r.DstPort = port.String()
	}
action:
	if lrate == 0 {
		return
	}
	r.Drop = ltype
	r.Rate = lrate
	return
}

func genL3l4Rule(ip, proto string, port common.PORT, isSrcPort bool, ltype int8, lrate uint64) (r common.Rule) {
	r.Type = "l3l4"
	r.SrcIP = ip
	r.IPProto = proto
	if proto == common.PROTO_STR_ICMP {
		goto action
	}
	if isSrcPort {
		r.SrcPort = port.String()
	} else {
		r.DstPort = port.String()
	}
action:
	if lrate == 0 {
		return
	}
	r.Drop = ltype
	r.Rate = lrate
	return
}

func batchAppendL3DropRules(rs *common.RuleSet, ipStat []common.IPCount) {
	var r common.Rule
	r.Type = "l3"
	r.Drop = common.ACTION_DROP
	for _, s := range ipStat {
		r.SrcIP = s.IP
		rs.Add(r)
	}
}

func batchAppendPortDropRules(rs *common.RuleSet, proto string, portStat []common.PortCount, isSrcPort bool) {
	for _, s := range portStat {
		rs.Add(genPortRule(proto, s.Port, isSrcPort, 0, 0))
	}
}
