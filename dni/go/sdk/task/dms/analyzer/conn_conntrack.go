package analyzer

import (
	"net"

	"github.com/amianetworks/dni/sdk/task/dms/common"
)

const (
	conntrackEstab    = "ESTABLISHED"
	conntrackClose    = "CLOSE"
	conntrackTimeWait = "TIME_WAIT"

	conntrackStatusUnreplied = "[UNREPLIED]"
	conntrackStatusAssured   = "[ASSURED]"
)

func ParseConntrack(ct *common.CtInfo, conntracks []map[string]string, ipNic map[string]string, filter map[string]struct{}) {
	for _, conntrack := range conntracks {
		switch conntrack["Proto"] {
		case "tcp":
			parseConntrackTCP(ct, conntrack, ipNic, filter)
		case "icmp":
			parseConntrackICMP(ct, conntrack, ipNic, filter)
		}
	}
}

func parseConntrackTCP(ct *common.CtInfo, conntrack map[string]string, ipNic map[string]string, filter map[string]struct{}) {
	if IsLocal(conntrack["oSIP"], ipNic) && IsLocal(conntrack["oDIP"], ipNic) {
		return
	}
	if ip := net.ParseIP(conntrack["oDIP"]); ip == nil || !ip.IsGlobalUnicast() {
		return
	}
	if IsLocal(conntrack["oSIP"], ipNic) {
		ct.Est[conntrack["oDIP"]] = struct{}{}
	} else if IsLocal(conntrack["oDIP"], ipNic) && conntrack["oDIP"] == conntrack["rSIP"] {
		if _, ok := filter[conntrack["oDIP"]]; !ok {
			return
		}
		switch conntrack["status"] {
		case conntrackEstab:
			ct.Est[conntrack["oSIP"]] = struct{}{}
		case conntrackTimeWait, conntrackClose:
			if conntrack["flag"] == conntrackStatusAssured {
				ct.Est[conntrack["oSIP"]] = struct{}{}
			}
		}

	} else if IsLocal(conntrack["rDIP"], ipNic) {
		ct.Nat["oSIP"] = struct{}{}
		ct.Nat["rSIP"] = struct{}{}
	} else {
		ct.Rtd["oSIP"] = struct{}{}
		ct.Rtd["oDIP"] = struct{}{}
	}
}

func parseConntrackICMP(ct *common.CtInfo, conntrack map[string]string, ipNic map[string]string, filter map[string]struct{}) {
	if IsLocal(conntrack["oSIP"], ipNic) && IsLocal(conntrack["oDIP"], ipNic) {
		return
	}
	if ip := net.ParseIP(conntrack["oDIP"]); ip == nil || !ip.IsGlobalUnicast() {
		return
	}
	if IsLocal(conntrack["oSIP"], ipNic) {
		ct.Est[conntrack["oDIP"]] = struct{}{}
	} else if IsLocal(conntrack["oDIP"], ipNic) && conntrack["oDIP"] == conntrack["rSIP"] {
		return
	} else if IsLocal(conntrack["rDIP"], ipNic) {
		ct.Nat["oSIP"] = struct{}{}
		ct.Nat["rSIP"] = struct{}{}
	} else {
		ct.Rtd["oSIP"] = struct{}{}
		ct.Rtd["oDIP"] = struct{}{}
	}
}

func IsLocal(ip string, ipNic map[string]string) bool {
	_, ok := ipNic[ip]
	return ok
}

func GetAddr(nicName string, nicIP map[string]string) string {
	ip := nicIP[nicName]
	return ip
}

func LookupDev(ip string, ipNic map[string]string) string {
	ip, ok := ipNic[ip]
	if !ok {
		return ""
	}
	return ip
}
