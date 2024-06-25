package analyzer

import (
	"fmt"
	"strings"

	"github.com/amianetworks/dni/sdk/task/dms/common"
)

type AT int

const (
	NOATTACK AT = iota
	ATTACK_ICMP
	ATTACK_TCP
	ATTACK_TCP_SYN
	ATTACK_TCP_ACK
	ATTACK_TCP_FIN
	ATTACK_TCP_RST
	ATTACK_UDP
	ATTACK_TCP_SEMI
	ATTACK_TCP_FULL
	ATTACK_HTTP_SLOW_READ
	ATTACK_REFLECT_UDP
	ATTACK_REFLECT_DNS
	ATTACK_REFLECT_NTP

	ATTACK_MAX
)

func (at AT) String() string {
	switch at {
	case ATTACK_ICMP:
		return "ICMP flood"
	case ATTACK_TCP:
		return "TCP flood"
	case ATTACK_TCP_SYN:
		return "TCP SYN flood"
	case ATTACK_TCP_ACK:
		return "TCP ACK flood"
	case ATTACK_TCP_FIN:
		return "TCP FIN flood"
	case ATTACK_TCP_RST:
		return "TCP RST flood"
	case ATTACK_TCP_SEMI:
		return "TCP semi-conn"
	case ATTACK_TCP_FULL:
		return "TCP full-conn"
	case ATTACK_HTTP_SLOW_READ:
		return "HTTP slow read"
	case ATTACK_UDP:
		return "UDP flood"
	case ATTACK_REFLECT_DNS:
		return "DNS reflect"
	case ATTACK_REFLECT_NTP:
		return "NTP reflect"
	case ATTACK_REFLECT_UDP:
		return "UDP reflect (from closed ports)"
	default:
		return fmt.Sprintf("UnknownAttackType%d", at)
	}
}

var flagmap = map[common.TCPFlag]AT{
	common.TCP_FLAG_SYN:    ATTACK_TCP_SYN,
	common.TCP_FLAG_SYNPSH: ATTACK_TCP_SYN,
	common.TCP_FLAG_SYNACK: ATTACK_TCP_ACK,
	common.TCP_FLAG_ACK:    ATTACK_TCP_ACK,
	common.TCP_FLAG_ACKPSH: ATTACK_TCP_ACK,
	common.TCP_FLAG_FIN:    ATTACK_TCP_FIN,
	common.TCP_FLAG_FINACK: ATTACK_TCP_ACK,
	common.TCP_FLAG_RST:    ATTACK_TCP_RST,
	common.TCP_FLAG_RSTACK: ATTACK_TCP_ACK,
}

// 分析器结果
type AttackInfo struct {
	Dev string //网卡名

	randSrc       bool //随机源
	randDstPort   bool //随机目标端口
	isDistributed bool //分散的，目标IP个数大于阈值

	// Attack Types
	Type       []float32 //attack类型
	IsAttacker bool      //	本地攻击
	Uncertain  bool      //攻击类型不确定

	// Attack Patterns
	//
	// attack patterns that can be directly mitigated. TODO: should change to
	// common.Match and let arbitrator decides
	Pattern common.Rules
	// attack patterns that may block benign traffic. TODO: should change to common.Match
	// and let arbitrator decides
	RiskyPattern common.Rules
	TCPConn      []*common.ConnInfo // long TCP connections TCP长连接
}

func (a *AttackInfo) String() string {
	buf := new(strings.Builder)
	if a.IsAttacker {
		buf.WriteString("This host is an attacker.\n")
	}
	fmt.Fprintf(buf, "Attack on dev %s detected: ", a.Dev)
	var n int
	for i, t := range a.Type {
		if AT(i) != ATTACK_TCP && t != 0 {
			if n != 0 {
				buf.WriteString(", ")
			}
			buf.WriteString(AT(i).String())
			n++
		}
	}
	if a.Noop() {
		return buf.String()
	}
	buf.WriteString("\nDetailed: ")
	if a.IsDistributed() {
		buf.WriteString("spoofed sources and dest ports")
	} else if a.randSrc {
		buf.WriteString("spoofed sources to local ports:")
		if a.Pattern != nil && !a.Pattern.Empty() {
			buf.WriteString(" closed:")
			for _, r := range a.Pattern.ToList() {
				fmt.Fprintf(buf, " %s", r.DstPort)
			}
			buf.WriteString(";")
		}
		if a.RiskyPattern != nil && !a.RiskyPattern.Empty() {
			buf.WriteString(" listened:")
			for _, r := range a.RiskyPattern.ToList() {
				fmt.Fprintf(buf, " %s", r.DstPort)
			}
		}
	} else {
		buf.WriteString("Attack patterns:")
		for _, r := range a.Pattern.ToList() {
			fmt.Fprintf(buf, " %s", r.SrcIP)
		}
	}
	return buf.String()
}

func (a *AttackInfo) UnderAttack() bool {
	for _, v := range a.Type {
		if v != 0 {
			return true
		}
	}
	return false
}

func (a *AttackInfo) IsEmpty() bool {
	return (a.Pattern == nil || a.Pattern.Empty()) && (a.RiskyPattern == nil || a.RiskyPattern.Empty()) &&
		len(a.TCPConn) == 0
}

func (a *AttackInfo) IsDistributed() bool {
	return a.isDistributed || (a.randSrc && a.randDstPort)
}

func (a *AttackInfo) Noop() bool {
	return !a.IsDistributed() && a.IsEmpty()
}

func makeAttackInfo() AttackInfo {
	return AttackInfo{Type: make([]float32, ATTACK_MAX)}
}

type Respond struct {
	Id int
	// Results
	AttackInfo
	// Detailed statistic data
	//
	// TODO: not implemented yet
	Reason interface{}
}

func newRespond() *Respond {
	return &Respond{AttackInfo: makeAttackInfo()}
}

// well-formatted for DMS diagnostic file
func (r *Respond) String() string {
	return r.AttackInfo.String()
}

// detailed information for developers
func (r *Respond) Verbose() string {
	var buf strings.Builder
	if r.IsAttacker {
		buf.WriteString("This host is an attacker.\n")
	}
	buf.WriteString("Estimated attack info:")
	var subStr strings.Builder
	var n int
	for i, t := range r.Type {
		if AT(i) != ATTACK_TCP && t != 0 {
			if n != 0 {
				subStr.WriteString(", ")
			}
			subStr.WriteString(AT(i).String())
			n++
		}
	}
	buf.WriteString(common.Indent(subStr.String()))
	buf.WriteString("\n")
	buf.WriteString("Source/Causes:\n")
	subStr.Reset()
	fmt.Fprintf(&subStr, "Patterns on dev %q: %v\n", r.Dev, r.Pattern)
	fmt.Fprintf(&subStr, "Ignored patterns on dev %q: %v\n", r.Dev, r.RiskyPattern)
	fmt.Fprintf(&subStr, "Num of suspicious conns: %v\n", len(r.TCPConn))
	buf.WriteString(common.Indent(subStr.String()))
	return buf.String()
}
