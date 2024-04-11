package utils

import (
	"encoding/binary"
	"net"
)

const (
	UINT32_MAX = ^uint32(0)
	UINT24_MAX = uint32(0xFFFFFF00)
	UINT16_MAX = uint32(0xFFFF0000)
)

// Ntoui converts IPv4 bytes to uint32
func Ntoui(b []byte) uint32 {
	return uint32(b[3]) | uint32(b[2])<<8 | uint32(b[1])<<16 | uint32(b[0])<<24
}

// Uiton converts a uint32 to IPv4 bytes
func Uiton(i uint32) []byte {
	return []byte{byte(i >> 24), byte(i >> 16), byte(i >> 8), byte(i)}
}

// Uitoa converts a uint32 represented IP to IPv4 string
func Uitoa(ip uint32) string {
	return net.IP(Uiton(ip)).String()
}

// Uitoin convets a uint32 and a mask to net.IPNet
func Uitoin(ip uint32, mask uint32) *net.IPNet {
	net := &net.IPNet{
		IP:   Uiton(ip),
		Mask: Uiton(mask),
	}
	return net
}

// Atoui converts an IPv4 string to uint32
func Atoui(ipstr string) uint32 {
	ip := net.ParseIP(ipstr).To4()
	if ip == nil {
		return 0
	}
	return binary.BigEndian.Uint32(ip)
}
