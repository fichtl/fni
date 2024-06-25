package abitrator

import (
	"github.com/amianetworks/dni/sdk/task/dms/analyzer"
	"github.com/amianetworks/dni/sdk/task/dms/assessor"
)

type intelligence struct {
	highSoft         bool
	highBps, highPps map[string]bool

	synCookieEnabled bool
}

func newIntelligence() *intelligence {
	return &intelligence{
		highBps: make(map[string]bool),
		highPps: make(map[string]bool),
	}
}

// dangerous indicates whether the high softirq is caused by this nic's rx
func (i *intelligence) dangerous(dev string) bool {
	// regard high bps as dangerous, though no valid mitigations

	// TODO: it's just a workaround in that now dms only sync already offloaded rules to
	// the controller. Should remove `i.highBps[dev]` case when intelligence get
	// correctly synced.
	return i.highSoft || i.highPps[dev] || i.highBps[dev]
}

var crossbarConn = map[assessor.ConnUtil]uint8{
	assessor.TCPCONN_ESTABLISHED: analyzer.CONN_TCP_ESTABLISHED,
	assessor.TCPCONN_TIME_WAIT:   analyzer.CONN_TCP_TIME_WAIT,
}

func GetQuery(input *assessor.AssessorInd) (*analyzer.Query, *intelligence) {
	if input.CPU.Normal() && input.BW.Normal() && input.SNMP.Normal() && input.TCPConn.Normal() {
		return nil, nil
	}

	var q = analyzer.NewQuery()
	var intelli = newIntelligence()
	intelli.highSoft = input.CPU.(assessor.CpuUtil).High()
	//proto bitmap
	var proto = input.SNMP.(assessor.SnmpUtil)
	//assessor proto bitmap => intelligence proto bitmap
	var m uint8
	if proto.Icmp() {
		m |= analyzer.P_ALL
	}
	if proto.Udp() {
		m |= analyzer.P_UDP
	}
	if proto.Tcp() {
		m |= analyzer.P_TCP
		if proto.TcpSyn() {
			// check if syn cookie is enabled
			intelli.synCookieEnabled = proto.TcpSynCookies()
		}
	}

	//snmp unnormal && (rx_bps | rx_pps unnormal)
	for dev, stat := range input.PerNicBW {
		if stat[assessor.NIC_RX_BYTES] && m != 0 {
			intelli.highBps[dev] = true
			q.SetDp(dev, m)
		}
		if stat[assessor.NIC_RX_PACKETS] && m != 0 {
			intelli.highPps[dev] = true
			q.SetDp(dev, m)
		}
	}
	//assessor tcpcoon status => anaylizer tcpconn status
	if !input.TCPConn.Normal() {
		conn := input.TCPConn.(assessor.ConnUtil)
		for in, out := range crossbarConn {
			if conn&in != 0 {
				q.Conn[analyzer.Measurement{L1: analyzer.TCPCONN, L2: out}] = struct{}{}
			}
		}
	}

	if q.IsNil() {
		return nil, intelli
	}
	return q, intelli
}
