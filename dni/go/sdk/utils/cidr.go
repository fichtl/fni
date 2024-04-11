package utils

import (
	"bytes"
	"fmt"
	"math/bits"
	"net"
)

// reference: https://github.com/zhanhb/cidr-merger.git
type IRange interface {
	ToIP() net.IP // return nil if can't be represented as a single ip
	ToIPNets() []*net.IPNet
	ToRange() *Range
	String() string
}

type Range struct {
	start net.IP
	end   net.IP
}

func (r *Range) ToIP() net.IP {
	if net.IP.Equal(r.start, r.end) {
		return r.start
	}
	return nil
}
func (r *Range) ToIPNets() []*net.IPNet {
	s, end := r.start, r.end
	ipBits := len(s) * 8
	var result []*net.IPNet
	for {
		// assert s <= end;
		cidr := max(prefixLength(xor(addOne(end), s)), ipBits-trailingZeros(s))
		ipNet := &net.IPNet{IP: s, Mask: net.CIDRMask(cidr, ipBits)}
		result = append(result, ipNet)
		tmp := lastIP(ipNet)
		if !lessThan(tmp, end) {
			return result
		}
		s = addOne(tmp)
	}
}

func (r *Range) ToRange() *Range {
	return r
}
func (r *Range) String() string {
	return fmt.Sprintf("%s-%s", r.start, r.end)
}

func lessThan(a, b net.IP) bool {
	if lenA, lenB := len(a), len(b); lenA != lenB {
		return lenA < lenB
	}
	return bytes.Compare(a, b) < 0
}

func max(a, b int) int {
	if a < b {
		return b
	}
	return a
}

func prefixLength(ip net.IP) int {
	for index, c := range ip {
		if c != 0 {
			return index*8 + bits.LeadingZeros8(c) + 1
		}
	}
	// special case for overflow
	return 0
}

func trailingZeros(ip net.IP) int {
	ipLen := len(ip)
	for i := ipLen - 1; i >= 0; i-- {
		if c := ip[i]; c != 0 {
			return (ipLen-i-1)*8 + bits.TrailingZeros8(c)
		}
	}
	return ipLen * 8
}

func lastIP(ipNet *net.IPNet) net.IP {
	ip, mask := ipNet.IP, ipNet.Mask
	ipLen := len(ip)
	if len(mask) != ipLen {
		panic("assert failed: unexpected IPNet " + ipNet.String())
	}
	res := make(net.IP, ipLen)
	for i := 0; i < ipLen; i++ {
		res[i] = ip[i] | ^mask[i]
	}
	return res
}

func addOne(ip net.IP) net.IP {
	ipLen := len(ip)
	res := make(net.IP, ipLen)
	for i := ipLen - 1; i >= 0; i-- {
		t := ip[i] + 1
		res[i] = t
		if t != 0 {
			copy(res, ip[0:i])
			break
		}
	}
	return res
}

func xor(a, b net.IP) net.IP {
	ipLen := len(a)
	res := make(net.IP, ipLen)
	for i := ipLen - 1; i >= 0; i-- {
		res[i] = a[i] ^ b[i]
	}
	return res
}
