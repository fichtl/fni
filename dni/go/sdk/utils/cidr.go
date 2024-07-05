package utils

import (
	"bytes"
	"fmt"
	"math"
	"math/bits"
	"net"
	"sort"
	"strings"
)

// reference: https://github.com/zhanhb/cidr-merger.git
type IRange interface {
	ToIP() net.IP // return nil if can't be represented as a single ip
	ToIPNets() []*net.IPNet
	ToRange() *Range
	String() string
}

func IPStrToIRange(text string) (IRange, error) {
	if index := strings.IndexByte(text, '/'); index != -1 {
		if _, network, err := net.ParseCIDR(text); err == nil {
			return IPNetWrapper{network}, nil
		} else {
			return nil, err
		}
	}
	if ip := net.ParseIP(text); ip != nil {
		return IPWrapper{ip}, nil
	}
	if index := strings.IndexByte(text, '-'); index != -1 {
		if start, end := net.ParseIP(text[:index]), net.ParseIP(text[index+1:]); start != nil && end != nil {
			if len(start) == len(end) && !lessThan(end, start) {
				return &Range{start: start, end: end}, nil
			}
		}
		return nil, &net.ParseError{Type: "range", Text: text}
	}
	return nil, &net.ParseError{Type: "ip/CIDR address/range", Text: text}
}

func IPNetToIRange(n IPNet) IRange {
	start := n.IPAddr & n.Mask
	end := n.IPAddr | ^n.Mask
	return &Range{net.IP(Uiton(start)), net.IP(Uiton(end))}
}

func IPNetSetToIRanges(ipnetSet *IPNetSet) []IRange {
	var arr []IRange
	if ipnetSet == nil {
		return arr
	}
	for n := range ipnetSet.Values() {
		arr = append(arr, IPNetToIRange(n))
	}
	return arr
}

func IRangeContains(a, b IRange) bool {
	ar, br := a.ToRange(), b.ToRange()
	return bytes.Compare(ar.start, br.start) <= 0 && bytes.Compare(ar.end, br.end) >= 0
}

func IRangesToIPNetSet(r []IRange) *IPNetSet {
	ipnetset := NewIPNetSet()
	for _, n := range r {
		for _, v := range n.ToIPNets() {
			ipnetset.Add(v.String())
		}
	}
	return ipnetset
}

type Range struct {
	start net.IP
	end   net.IP
}

func (r *Range) familyLength() int {
	return len(r.start)
}
func (r *Range) contains(a uint32) bool {
	b := Uiton(a)
	return bytes.Compare(r.start, b) <= 0 && bytes.Compare(b, r.end) <= 0
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

func IPNetToRange(n IPNet) *Range {
	start := n.IPAddr & n.Mask
	end := n.IPAddr | ^n.Mask
	return &Range{net.IP(Uiton(start)), net.IP(Uiton(end))}
}

type IPWrapper struct {
	net.IP
}

func (r IPWrapper) ToIP() net.IP {
	return r.IP
}
func (r IPWrapper) ToIPNets() []*net.IPNet {
	ipBits := len(r.IP) * 8
	return []*net.IPNet{
		{IP: r.IP, Mask: net.CIDRMask(ipBits, ipBits)},
	}
}
func (r IPWrapper) ToRange() *Range {
	return &Range{start: r.IP, end: r.IP}
}

type IPNetWrapper struct {
	*net.IPNet
}

func (r IPNetWrapper) ToIP() net.IP {
	if allFF(r.Mask) {
		return r.IP
	}
	return nil
}
func (r IPNetWrapper) ToIPNets() []*net.IPNet {
	return []*net.IPNet{r.IPNet}
}
func (r IPNetWrapper) ToRange() *Range {
	ipNet := r.IPNet
	return &Range{start: ipNet.IP, end: lastIP(ipNet)}
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

func allFF(ip []byte) bool {
	for _, c := range ip {
		if c != 0xff {
			return false
		}
	}
	return true
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

func convertBatch(wrappers []IRange) []IRange {
	result := make([]IRange, 0, len(wrappers))
	for _, r := range wrappers {
		for _, ipNet := range r.ToIPNets() {
			// can't use range iterator, for operator address of is taken
			// it seems a trick of golang here
			result = append(result, IPNetWrapper{ipNet})
		}
	}

	return result
}

type Ranges []*Range

func (s Ranges) Len() int { return len(s) }
func (s Ranges) Swap(i, j int) {
	s[i], s[j] = s[j], s[i]
}
func (s Ranges) Less(i, j int) bool {
	return lessThan(s[i].start, s[j].start)
}

func sortAndMerge(wrappers []IRange) []IRange {
	// assume len(wrappers) > 1
	if len(wrappers) == 0 {
		res := make([]IRange, 0)
		return res
	}
	ranges := make([]*Range, 0, len(wrappers))
	for _, e := range wrappers {
		ranges = append(ranges, e.ToRange())
	}
	sort.Sort(Ranges(ranges))

	res := make([]IRange, 0, len(ranges))
	now := ranges[0]
	familyLength := now.familyLength()
	start, end := now.start, now.end
	for i, count := 1, len(ranges); i < count; i++ {
		now := ranges[i]
		if fl := now.familyLength(); fl != familyLength {
			res = append(res, &Range{start, end})
			familyLength = fl
			start, end = now.start, now.end
			continue
		}
		if allFF(end) || !lessThan(addOne(end), now.start) {
			if lessThan(end, now.end) {
				end = now.end
			}
		} else {
			res = append(res, &Range{start, end})
			start, end = now.start, now.end
		}
	}
	return append(res, &Range{start, end})
}

func SingleOrSelf(r IRange) IRange {
	if ip := r.ToIP(); ip != nil {
		return IPWrapper{ip}
	}
	return r
}

func MergeIRanges(in []IRange) []IRange {
	ret := sortAndMerge(in)
	return convertBatch(ret)
}

func maskToCidr(mask uint32) int {
	if mask == 0 {
		return 0
	}
	return 32 - int(math.Log2(float64(^mask+1)))
}

func getMaskLen(mask uint32) uint8 {
	var count uint8
	for mask != 0 {
		count++
		mask = mask & (mask - 1)
	}
	return count
}

// compareCIDR returns an integer comparing two cidr strings. The result will be 0 if a
// equals b,-2 if a is unrelated to b, -1 if b contains a, and +1 if a contains b. A nil
// argument is equivalent to an empty slice.
func compareCIDR(a, b IPNet) int {
	if a == b {
		return 0
	}

	var ar, br *Range
	ar = IPNetToRange(a)
	br = IPNetToRange(b)

	res := bytes.Compare(ar.end, br.end) - bytes.Compare(ar.start, br.start)
	if res == 0 {
		if ar.end.Equal(br.end) {
			return 0
		} else {
			return -2
		}
	} else if res < 0 {
		return -1
	} else {
		return +1
	}
}

func splitCIDR(a, b IPNet) []string {
	var ret []string
	var left, right Range

	r := IPNetToRange(a)
	left.start = r.start
	right.end = r.end
	r = IPNetToRange(b)
	left.end = r.start
	right.start = r.end

	net1 := left.ToIPNets()
	net2 := right.ToIPNets()
	ret = make([]string, len(net1)+len(net2))
	for i, n := range net1 {
		ret[i] = n.String()
	}
	for i, n := range net2 {
		ret[len(net1)+i] = n.String()
	}
	return ret
}

func removeRange(curr []IRange, new IRange) ([]IRange, []IRange) {
	return nil, nil
}

func RemoveRange(in []IRange, out []IRange) []IRange {

	in = sortAndMerge(in)
	out = sortAndMerge(out)
	// in and out are sorted in ascending range order, and have no overlaps within each
	// other. We can run a merge of the two lists in one pass.

	min := make([]IRange, 0, len(in))
	for len(in) > 0 && len(out) > 0 {
		rin, rout := in[0], out[0]
		switch {
		case bytes.Compare(rout.ToRange().end, rin.ToRange().start) < 0:
			// "out" is entirely before "in".
			//
			//    out         in
			// f-------t   f-------t
			out = out[1:] //out before int just drop out
		case bytes.Compare(rin.ToRange().end, rout.ToRange().start) < 0:
			// "in" is entirely before "out".
			//
			//    in         out
			// f------t   f-------t
			min = append(min, rin)
			in = in[1:] //in before out just append in
		case bytes.Compare(rout.ToRange().start, rin.ToRange().start) <= 0 &&
			bytes.Compare(rout.ToRange().end, rin.ToRange().end) >= 0:
			// "out" entirely covers "in".
			//
			//       out
			// f-------------t
			//    f------t
			//       in
			in = in[1:]
		case bytes.Compare(rin.ToRange().start, rout.ToRange().start) <= 0 &&
			bytes.Compare(rin.ToRange().end, rout.ToRange().end) >= 0:
			// "in" entirely covers "out".
			//
			//       in
			// f-------------t
			//    f------t
			//       out
			//
			// Adjust in[0], not ir, because we want to consider the mutated range on the
			// next iteration.
			rout.ToRange().start.To4()[3] -= 1 //3 refer to the last sec of ip sections
			rout.ToRange().end.To4()[3] += 1
			min = append(min, &Range{start: rin.ToRange().start, end: rout.ToRange().start})
			in[0].ToRange().start = rout.ToRange().end
			out = out[1:]
		case bytes.Compare(rout.ToRange().start, rin.ToRange().start) <= 0 &&
			bytes.Compare(rin.ToRange().end, rout.ToRange().end) >= 0:
			// "out" overlaps start of "in".
			//
			//   out
			// f------t
			//    f------t
			//       in
			//
			// Can't move ir onto min yet, another later out might trim it further. Just
			// discard or and continue.
			rout.ToRange().end.To4()[3] += 1
			in[0].ToRange().start = rout.ToRange().end
			out = out[1:]
		case bytes.Compare(rin.ToRange().start, rout.ToRange().start) < 0 &&
			bytes.Compare(rout.ToRange().end, rin.ToRange().end) > 0:
			// "out" overlaps end of "in".
			//
			//           out
			//        f------t
			//    f------t
			//       in
			rout.ToRange().start.To4()[3] -= 1
			min = append(min, &Range{start: rin.ToRange().start, end: rout.ToRange().start})
			in = in[1:]
		default:
			panic("unexpected additional overlap scenario")
		}
	}
	if len(in) > 0 {
		// Ran out of removals before the end of in.
		min = append(min, in...)
	}
	return min
}
