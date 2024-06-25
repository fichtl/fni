package common

import "regexp"

const (
	B uint64 = 1 << (iota * 10)
	KB
	MB
	GB
	TB
)

const (
	IP_HEADER_MIN_LEN  int = 20
	TCP_HEADER_MIN_LEN int = 20
	UDP_HEADER_MIN_LEN int = 8
	DNS_HEADER_MIN_LEN int = 12
)

const (
	PROTO_STR_TCP  = "tcp"
	PROTO_STR_UDP  = "udp"
	PROTO_STR_ICMP = "icmp"
)

var (
	lineStart = regexp.MustCompile(`(?m)^`)
)

func Indent(in string) string {
	return lineStart.ReplaceAllString(in, "\t")
}
