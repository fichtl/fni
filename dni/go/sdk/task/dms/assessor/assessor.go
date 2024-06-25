package assessor

import (
	"strings"

	"github.com/amianetworks/dni/sdk/task/dms/common"
	config "github.com/amianetworks/dni/sdk/task/dms/config"
)

type UtilInd interface {
	String() string
	Normal() bool
}

// CPU
type CpuUtil uint64

const (
	CPU_UNINIT CpuUtil = iota
	CPU_LOW
	CPU_M_LOW
	CPU_M_HIGH
	CPU_HIGH
)

func (u CpuUtil) String() string {
	switch u {
	case CPU_UNINIT:
		return "Uninitialized"
	case CPU_LOW:
		return "Low"
	case CPU_M_LOW:
		return "ModerateLow"
	case CPU_M_HIGH:
		return "ModerateHigh"
	case CPU_HIGH:
		return "High"
	default:
		return "Uninitialized"
	}
}

func (u CpuUtil) Normal() bool {
	return u <= CPU_M_LOW
}

func (u CpuUtil) High() bool {
	return u == CPU_HIGH
}

func CalcCPUUtil(c float64, th config.CPUUtilThreshold) CpuUtil {
	switch {
	case c <= th.Low:
		return CPU_LOW
	case c <= th.Moderate:
		return CPU_M_LOW
	case c <= th.High:
		return CPU_M_HIGH
	case c > th.High:
		return CPU_HIGH
	}
	return CPU_LOW
}

// BW
type BwUtil uint64

const (
	BW_LOW         BwUtil = iota
	BW_RX_BPS_HIGH BwUtil = 1 << (iota - 1)
	BW_RX_PPS_HIGH
	BW_TX_BPS_HIGH
	BW_TX_PPS_HIGH

	bwBitsize int = iota
)

func (u BwUtil) String() string {
	if u == BW_LOW {
		return "Normal"
	}
	var buf strings.Builder
	var f BwUtil
	var n int
	for i := 0; i < bwBitsize; i++ {
		f = 1 << i
		if u&f == f {
			if n != 0 {
				buf.WriteString(", ")
			}
			switch f {
			case BW_RX_BPS_HIGH:
				buf.WriteString("RxBPSHigh")
			case BW_RX_PPS_HIGH:
				buf.WriteString("RxPPSHigh")
			case BW_TX_BPS_HIGH:
				buf.WriteString("TxBPSHigh")
			case BW_TX_PPS_HIGH:
				buf.WriteString("TxPPSHigh")
			}
			n++
		}
	}
	return buf.String()
}

func (u BwUtil) Normal() bool {
	return u == BW_LOW
}

const (
	NIC_RX_BYTES   = "rx_bytes"
	NIC_RX_PACKETS = "rx_packets"
	NIC_TX_BYTES   = "tx_bytes"
	NIC_TX_PACKETS = "tx_packets"
)

func CalcBWUtil(bw map[string]map[string]uint64, speed map[string]uint64, th config.BwUtilThreshold) (BwUtil, map[string]map[string]bool) {
	//every netif BwStat
	statusMap := make(map[string]map[string]bool)
	shapeResult := BW_LOW
	thbps := uint64(th.BPS)
	thpps := uint64(th.PPS)
	for dev, data := range bw {
		var status BwUtil
		statusMap[dev] = make(map[string]bool)
		if 100*(data[NIC_RX_BYTES]*8/common.MB)/speed[dev] >= thbps {
			status |= BW_RX_BPS_HIGH
			statusMap[dev][NIC_RX_BYTES] = true
		}
		if data[NIC_RX_PACKETS] >= thpps {
			status |= BW_RX_PPS_HIGH
			statusMap[dev][NIC_RX_PACKETS] = true
		}
		if 100*(data[NIC_TX_BYTES]*8/common.MB)/speed[dev] >= thbps {
			status |= BW_TX_BPS_HIGH
			statusMap[dev][NIC_TX_BYTES] = true
		}
		if data[NIC_TX_PACKETS] >= thpps {
			status |= BW_TX_PPS_HIGH
			statusMap[dev][NIC_TX_PACKETS] = true
		}
		shapeResult |= status
	}
	return shapeResult, statusMap
}

// SNMP
type SnmpUtil uint64

// Keep the following two constants defined in the same order
// index of different type of statistic in statistic array `statArray'
const (
	PROTO_NORMAL SnmpUtil = iota
	PROTO_IP     SnmpUtil = 1 << (iota - 1)
	PROTO_ICMP
	PROTO_TCP_SYN1
	PROTO_TCP_SYN2
	PROTO_TCP_OUTRST
	PROTO_TCP_OTHERS
	PROTO_UDP_OPENED
	PROTO_UDP_CLOSED
	PROTO_UDPLITE
	PROTO_NONIPV4

	snmpBitsize int = iota
)

const (
	PROTO_TCP_SYN = PROTO_TCP_SYN1 | PROTO_TCP_SYN2
	PROTO_TCP     = PROTO_TCP_SYN | PROTO_TCP_OUTRST | PROTO_TCP_OTHERS
	PROTO_UDP     = PROTO_UDP_OPENED | PROTO_UDP_CLOSED
)

const (
	thReversePath = 100_000 // ad-hoc
)

func (u SnmpUtil) String() string {
	if u == 0 {
		return "Normal"
	}
	var buf strings.Builder
	var f SnmpUtil
	var n int
	for i := 0; i < snmpBitsize; i++ {
		f = (1 << i)
		if u&f == f {
			if n != 0 {
				buf.WriteString(", ")
			}
			switch f {
			case PROTO_IP:
				buf.WriteString("IP")
			case PROTO_ICMP:
				buf.WriteString("ICMP")
			case PROTO_TCP_SYN1, PROTO_TCP_SYN2:
				buf.WriteString("TCP-SYN")
			case PROTO_TCP_OUTRST:
				buf.WriteString("TCP-OUTRST")
			case PROTO_TCP_OTHERS:
				buf.WriteString("TCP-OTHERS")
			case PROTO_UDP_OPENED:
				buf.WriteString("UDP")
			case PROTO_UDP_CLOSED:
				buf.WriteString("UDP-CLOSED")
			case PROTO_UDPLITE:
				buf.WriteString("UDPLITE")
			case PROTO_NONIPV4:
				buf.WriteString("NONIPV4")
			default:
				buf.WriteString("UNEXPECTED")
			}
			n++
		}
	}
	return buf.String()
}

func (u SnmpUtil) Normal() bool {
	return u == PROTO_NORMAL
}

func (u SnmpUtil) Icmp() bool {
	return u&PROTO_ICMP == PROTO_ICMP
}

func (u SnmpUtil) TcpSyn() bool {
	return u&PROTO_TCP_SYN != 0
}
func (u SnmpUtil) Tcp() bool {
	return u&PROTO_TCP != 0
}
func (u SnmpUtil) TcpSynCookies() bool {
	return u&PROTO_TCP_SYN2 != 0
}

func (u SnmpUtil) Udp() bool {
	return u&PROTO_UDP != 0
}

func CheckIcmp(data map[string]uint64, freq uint64, pps config.ProtoThreshold) SnmpUtil {
	th := uint64(pps.GetPPSThreshold("icmp", true)) * freq
	var brief SnmpUtil
	if data["IcmpInMsgs"] >= th {
		brief |= PROTO_ICMP
	}
	return brief
}

func CheckTcp(data map[string]uint64, freq uint64, pps config.ProtoThreshold) SnmpUtil {
	th := uint64(pps.GetPPSThreshold("tcp", true)) * freq
	var brief SnmpUtil
	// TCP syn flood (no syncookies)
	if data["TcpExtTCPReqQFullDrop"] >= th {
		brief |= PROTO_TCP_SYN1
	}
	// TCP syn flood (syncookies)
	if data["TcpExtTCPReqQFullDoCookies"] >= th {
		brief |= PROTO_TCP_SYN2
	}
	// TCP ack flood or TCP on closed port
	if data["TcpOutRsts"] >= th {
		brief |= PROTO_TCP_OUTRST
	}
	// other TCP-related floods, e.g. fin/rst flood
	if data["TcpInSegs"]-(data["TcpExtTCPReqQFullDrop"]+data["TcpExtTCPReqQFullDoCookies"]+data["TcpOutRsts"]) >= th {
		brief |= PROTO_TCP_OTHERS
	}
	return brief
}

func CheckUdp(data map[string]uint64, freq uint64, pps config.ProtoThreshold) SnmpUtil {
	th := uint64(pps.GetPPSThreshold("udp", true)) * freq
	var brief SnmpUtil
	if data["UdpRcvbufErrors"] >= th {
		brief |= PROTO_UDP_OPENED
	}
	if data["UdpNoPorts"] >= th {
		brief |= PROTO_UDP_CLOSED
	}
	return brief
}

func CalcSnmpUtil(data map[string]uint64, freq uint64, pps config.ProtoThreshold, protoProhibit map[string]bool) SnmpUtil {
	rp := data["TcpExtIPReversePathFilter"] > (thReversePath * freq)
	var brief SnmpUtil
	if !protoProhibit["icmp"] {
		if rp {
			brief |= PROTO_ICMP
		} else {
			brief |= CheckIcmp(data, freq, pps)
		}
	}
	if !protoProhibit["tcp"] {
		if rp {
			brief |= PROTO_TCP
		} else {
			brief |= CheckTcp(data, freq, pps)
		}
	}
	if !protoProhibit["udp"] {
		if rp {
			brief |= PROTO_UDP
		} else {
			brief |= CheckUdp(data, freq, pps)
		}
	}
	return brief
}

// TCPCONN
type ConnUtil uint64

// indicators that mark abnormal of different tcp connection states
const (
	TCPCONN_NORMAL      ConnUtil = iota
	TCPCONN_ESTABLISHED ConnUtil = 1 << (iota - 1)
	TCPCONN_SYN_SENT
	TCPCONN_SYN_RECV
	TCPCONN_FIN_WAIT1
	TCPCONN_FIN_WAIT2
	TCPCONN_TIME_WAIT
	TCPCONN_CLOSE
	TCPCONN_CLOSE_WAIT
	TCPCONN_LAST_ACK
	TCPCONN_LISTEN
	TCPCONN_CLOSING

	connBitsize int = int(common.TCP_MAX_STATES)
)

func (u ConnUtil) String() string {
	if u == TCPCONN_NORMAL {
		return "Normal"
	}
	var buf strings.Builder
	var f ConnUtil
	var n int
	for i := 0; i < connBitsize; i++ {
		f = 1 << i
		if u&f == f {
			if n != 0 {
				buf.WriteString(",")
			}
			switch f {
			case TCPCONN_ESTABLISHED:
				buf.WriteString("ESTAB")
			case TCPCONN_SYN_SENT:
				buf.WriteString("SYN-SENT")
			case TCPCONN_SYN_RECV:
				buf.WriteString("SYN-RECV")
			case TCPCONN_FIN_WAIT1:
				buf.WriteString("FIN-WAIT-1")
			case TCPCONN_FIN_WAIT2:
				buf.WriteString("FIN-WAIT-2")
			case TCPCONN_TIME_WAIT:
				buf.WriteString("TIME-WAIT")
			case TCPCONN_CLOSE:
				buf.WriteString("UNCONN")
			case TCPCONN_CLOSE_WAIT:
				buf.WriteString("CLOSE-WAIT")
			case TCPCONN_LAST_ACK:
				buf.WriteString("LAST-ACK")
			case TCPCONN_LISTEN:
				buf.WriteString("LISTEN")
			case TCPCONN_CLOSING:
				buf.WriteString("CLOSING")
			default:
				buf.WriteString("UNKNOWN")
			}
			n++
		}
	}
	return buf.String()
}

func (u ConnUtil) Normal() bool {
	return u == TCPCONN_NORMAL
}

func CalcTCPConn(connStats map[string]uint64, tcpconnTh config.TcpConnUtilThreshold) ConnUtil {
	var status ConnUtil
	if connStats["TCP_SYN_RECV"] >= uint64(tcpconnTh.SemiConn) {
		status |= TCPCONN_SYN_RECV
	}

	// full-conn flood, slow attacks
	if connStats["TCP_ESTABLISHED"] >= uint64(tcpconnTh.FullConn) {
		status |= TCPCONN_ESTABLISHED
	}

	// outdated connection, server recycle connection
	if connStats["TCP_TIME_WAIT"] >= uint64(tcpconnTh.FullConn) {
		status |= TCPCONN_TIME_WAIT
	}
	return status
}

type AssessorInd struct {
	CPU      UtilInd // CPU utilization indicator (index)
	SNMP     UtilInd // Proto rate status indicator (bitmap)
	TCPConn  UtilInd // TCPConn status indicator (bitmap)
	BW       UtilInd // Bandwidth status indicator (bitmap)
	PerNicBW map[string]map[string]bool
}

type AssessorData struct {
	CPU      float64
	BW       map[string]map[string]uint64
	TCPConn  map[string]uint64
	SNMP     map[string]uint64
	NicSpeed map[string]uint64
}
