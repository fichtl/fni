package common

import (
	"strings"
)

// Connection state is an enum that follows the order defined below
const (
	TCP_UNKNOWN uint8 = iota
	TCP_ESTABLISHED
	TCP_SYN_SENT
	TCP_SYN_RECV
	TCP_FIN_WAIT1
	TCP_FIN_WAIT2
	TCP_TIME_WAIT
	TCP_CLOSE
	TCP_CLOSE_WAIT
	TCP_LAST_ACK
	TCP_LISTEN
	TCP_CLOSING
	TCP_MAX_STATES
)

var stateName = [...]string{
	TCP_UNKNOWN:     "UNKNOWN",
	TCP_ESTABLISHED: "ESTAB",
	TCP_SYN_SENT:    "SYN-SENT",
	TCP_SYN_RECV:    "SYN-RECV",
	TCP_FIN_WAIT1:   "FIN-WAIT-1",
	TCP_FIN_WAIT2:   "FIN-WAIT-2",
	TCP_TIME_WAIT:   "TIME-WAIT",
	TCP_CLOSE:       "UNCONN",
	TCP_CLOSE_WAIT:  "CLOSE-WAIT",
	TCP_LAST_ACK:    "LAST-ACK",
	TCP_LISTEN:      "LISTEN",
	TCP_CLOSING:     "CLOSING",
}

var revState = map[string]uint8{
	"UNKNOWN":    TCP_UNKNOWN,
	"ESTAB":      TCP_ESTABLISHED,
	"SYN-SENT":   TCP_SYN_SENT,
	"SYN-RECV":   TCP_SYN_RECV,
	"FIN-WAIT-1": TCP_FIN_WAIT1,
	"FIN-WAIT-2": TCP_FIN_WAIT2,
	"TIME-WAIT":  TCP_TIME_WAIT,
	"UNCONN":     TCP_CLOSE,
	"CLOSE-WAIT": TCP_CLOSE_WAIT,
	"LAST-ACK":   TCP_LAST_ACK,
	"LISTEN":     TCP_LISTEN,
	"CLOSING":    TCP_CLOSING,
}

// TCP flags
const (
	TCP_FLAG_FIN TCPFlag = 1 << iota
	TCP_FLAG_SYN
	TCP_FLAG_RST
	TCP_FLAG_PSH
	TCP_FLAG_ACK
	TCP_FLAG_URG
	TCP_FLAG_ECE // rfc 3168
	TCP_FLAG_CWR // rfc 3168
	// TCP_FLAG_NS  // rfc 3540

	tcpFlagSize int = iota
)

const (
	TCP_FLAG_SYNACK = TCP_FLAG_SYN | TCP_FLAG_ACK
	TCP_FLAG_FINACK = TCP_FLAG_FIN | TCP_FLAG_ACK
	TCP_FLAG_RSTACK = TCP_FLAG_RST | TCP_FLAG_ACK
	TCP_FLAG_ACKPSH = TCP_FLAG_PSH | TCP_FLAG_ACK
	TCP_FLAG_SYNPSH = TCP_FLAG_PSH | TCP_FLAG_SYN
)

type TCPFlag uint8

func (f TCPFlag) String() string {
	if f == 0 {
		return "NULL"
	}
	var buf strings.Builder
	var ff TCPFlag
	var n int
	for i := 0; i < tcpFlagSize; i++ {
		ff = (1 << i)
		if f&ff == ff {
			if n != 0 {
				buf.WriteString(",")
			}
			switch ff {
			case TCP_FLAG_FIN:
				buf.WriteString("FIN")
			case TCP_FLAG_SYN:
				buf.WriteString("SYN")
			case TCP_FLAG_RST:
				buf.WriteString("RST")
			case TCP_FLAG_PSH:
				buf.WriteString("PSH")
			case TCP_FLAG_ACK:
				buf.WriteString("ACK")
			case TCP_FLAG_URG:
				buf.WriteString("URG")
			case TCP_FLAG_ECE:
				buf.WriteString("ECE")
			case TCP_FLAG_CWR:
				buf.WriteString("CWR")
			}
			n++
		}
	}
	return buf.String()
}
