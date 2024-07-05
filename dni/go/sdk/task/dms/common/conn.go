package common

import (
	"fmt"
	"strconv"
	"strings"
)

type PortInfo struct {
	// Actively listening ports
	Listen map[PORT]struct{}
	// Local ports of established connections, as well as peer IP addresses
	Local map[PORT]map[string]struct{}
	// Remote ports of established connections
	Remote map[PORT]struct{}
}

func NewPortInfo() *PortInfo {
	return &PortInfo{
		make(map[PORT]struct{}),
		make(map[PORT]map[string]struct{}),
		make(map[PORT]struct{}),
	}
}

func (pi *PortInfo) String() string {
	var buf strings.Builder
	fmt.Fprintf(&buf, "Listening: %+v", pi.Listen)
	return buf.String()
}

type FlowType int64

const (
	FlowTypeUnkonwn FlowType = iota
	FlowTypeTcp
	FlowTypeUdp
	FlowTypeIcmp
)

func (f FlowType) String() string {
	switch f {
	case FlowTypeTcp:
		return "tcp"
	case FlowTypeUdp:
		return "udp"
	case FlowTypeIcmp:
		return "icmp"
	default:
		return "unknown"
	}
}

type Flow struct {
	Type         FlowType
	Rip, Lip     string
	Rport, Lport PORT
}

func (f Flow) String() string {
	return fmt.Sprintf("%s:%d->%s:%d", f.Lip, f.Lport, f.Rip, f.Rport)
}

// FilterExp generates ss-compatible filter string that matches the Flow.
func (f Flow) FilterExp() string {
	var out = new(strings.Builder)
	switch f.Type {
	case FlowTypeTcp:
		out.WriteString("--tcp")
	case FlowTypeUdp:
		out.WriteString("--udp")
	default:
		return ""
	}
	n := out.Len()
	if f.Lip != "" {
		fmt.Fprintf(out, " src %s", f.Lip)
	}
	if f.Lport != 0 {
		fmt.Fprintf(out, ":%d", f.Lport)
	}
	if f.Rip != "" {
		fmt.Fprintf(out, " dst %s", f.Rip)
	}
	// if f.Rport != 0 {
	// 	fmt.Fprintf(out, " dport %d", f.Rport)
	// }
	if out.Len() == n { // all of above fields are zero value
		return ""
	}
	return out.String()
}

type ConnInfo struct {
	// flow information
	Src, Dst string
	Name     string

	// send/recv buffer information
	Send, Recv   int
	SPort, DPort int
}

func (ci *ConnInfo) String() string {
	return ci.Name
}

func (ci *ConnInfo) Flow() (f Flow) {
	f.Lip = ci.Src
	f.Lport = PORT(ci.SPort)
	f.Rip = ci.Dst
	f.Rport = PORT(ci.DPort)
	return
}

func parseIPPortString(token string) (string, uint16, error) {
	if strings.HasPrefix(token, "[") {
		return "", 0, fmt.Errorf("IPv6 address not supported yet: %q", token)
	}
	s := strings.Split(token, ":")
	if len(s) != 2 {
		return "", 0, fmt.Errorf("malformed IP:Port: %q", token)
	}
	port, err := strconv.ParseUint(s[1], 10, 16)
	if err != nil {
		return "", 0, err
	}
	ip := s[0]
	// For SO_BINDTODEVICE socket that contains `%' in address
	if idx := strings.Index(ip, "%"); idx >= 0 {
		ip = ip[:idx]
	}
	return ip, uint16(port), nil
}

type CtInfo struct {
	SynRcv    map[string]int
	Unreplied map[string]int

	Est map[string]struct{}
	// EstIgr, EstEgr FlowSet // detailed, future use

	EstTcp map[PORT]map[string]struct{}
	EstUdp map[PORT]map[string]struct{}

	Nat map[string]struct{}
	// PNat map[PORT]FlowSet // detailed, future use

	Rtd map[string]struct{}
	// RtdFlow FlowSet // detailed, future use
}

func NewCtInfo() *CtInfo {
	return &CtInfo{
		SynRcv:    make(map[string]int),
		Unreplied: make(map[string]int),

		Est: make(map[string]struct{}),
		// EstIgr: make(FlowSet), EstEgr: make(FlowSet),

		EstTcp: make(map[PORT]map[string]struct{}),
		EstUdp: make(map[PORT]map[string]struct{}),

		Nat: make(map[string]struct{}),
		// PNat: make(map[PORT]FlowSet),

		Rtd: make(map[string]struct{}),
		// RtdFlow: make(FlowSet),
	}
}

func (ct *CtInfo) String() string {
	buf := new(strings.Builder)
	// fmt.Fprintf(buf, "SynRecv IP: %+v\n", ct.SynRcv)
	fmt.Fprintf(buf, "Estab IP: %+v\n", ct.Est)
	// fmt.Fprintf(buf, "EstIngress: %+v\n", ct.EstIgr)
	// fmt.Fprintf(buf, "EstEgress: %+v\n", ct.EstEgr)
	fmt.Fprintf(buf, "Estab Tcp: %+v\n", ct.EstTcp)
	// fmt.Fprintf(buf, "Estab Udp: %+v\n", ct.EstUdp)
	fmt.Fprintf(buf, "NATed IP: %+v\n", ct.Nat)
	// fmt.Fprintf(buf, "Per-port NATed: %+v\n", ct.PortNat)
	fmt.Fprintf(buf, "FWDed IP: %+v\n", ct.Rtd)
	// fmt.Fprintf(buf, "RtdFlow: %+v\n", ct.RtdFlow)
	fmt.Fprintf(buf, "#Unrep IP: %d", len(ct.Unreplied))
	return buf.String()
}
