package analyzer

import (
	"fmt"
	"strings"
)

// L1
const (
	CPU     uint8 = iota // cpu resources
	BW                   // bandwidth
	PROTO                // packet per second
	TCPCONN              // tcp connections
	// append new measurement here
	MAX_L1_MEASUREMENT
)

// L2
const (
	CPU_TIME uint8 = iota
	// CPU_TOP
	CPU_ALL
)

const (
	BW_RX_BPS uint8 = iota
	BW_RX_PPS
	BW_TX_BPS
	BW_TX_PPS
	BW_ALL
)

const (
	P_ICMP uint8 = 1 << iota
	P_TCP
	P_UDP
	P_UDPLITE
	P_NONIPV4

	protoBitsize = iota
)

const P_ALL = P_ICMP | P_TCP | P_UDP

var protoString = [protoBitsize]string{"icmp", "tcp", "udp", "udplite", "nonipv4"}

const (
	CONN_TCP_SYN uint8 = iota
	CONN_TCP_ESTABLISHED
	CONN_TCP_TIME_WAIT
	CONN_TCP_CTIME
	CONN_ALL
)

type Measurement struct {
	L1 uint8
	L2 uint8
}

func (m Measurement) String() string {
	var l1, l2 string
	switch m.L1 {
	case CPU:
		l1 = "CPU"
		switch m.L2 {
		case CPU_TIME:
			l2 = "time"
		case CPU_ALL:
			l2 = "all"
		}
	case BW:
		l1 = "Bandwidth"
		switch m.L2 {
		case BW_RX_BPS:
			l2 = "rx bps"
		case BW_RX_PPS:
			l2 = "rx pps"
		case BW_TX_BPS:
			l2 = "tx bps"
		case BW_TX_PPS:
			l2 = "tx pps"
		case BW_ALL:
			l2 = "all"
		}
	case PROTO:
		l1 = "Proto"
		protos := ParseProtoIndicator(m.L2)
		l2 = strings.Join(protos, " ")
	case TCPCONN:
		l1 = "TCPConn"
		switch m.L2 {
		case CONN_TCP_SYN:
			l2 = "handshake"
		case CONN_TCP_ESTABLISHED:
			l2 = "established"
		case CONN_TCP_TIME_WAIT:
			l2 = "timewait"
		case CONN_TCP_CTIME:
			l2 = "ctime"
		case CONN_ALL:
			l2 = "all"
		}
	}
	return fmt.Sprintf("%s (%s)", l1, l2)
}

func ParseProtoIndicator(m uint8) []string {
	if m == P_ALL {
		return []string{"all"}
	}
	var ind uint8
	var buf = make([]string, 0)
	for i := 0; i < protoBitsize; i++ {
		ind = 1 << i
		if m&ind != ind {
			continue
		}
		buf = append(buf, protoString[i])
	}
	return buf
}
