package common

import (
	"bufio"
	"bytes"
	"encoding/binary"
	"encoding/json"
	"fmt"
	"net"
	"os"
	"sort"
	"strconv"
)

const (
	UINT64_MAX = ^uint64(0)
	UINT32_MAX = ^uint32(0)
)

// Self-defined IPNet, net.IPNet is not comparable
//
// TODO: use netip.Addr (introduced after go1.18)
type IPNet struct {
	IPAddr uint32
	Mask   uint32
}

func (n IPNet) String() string {
	if n.Mask == UINT32_MAX {
		return Uitoa(n.IPAddr)
	}
	return Uitoa(n.IPAddr) + "/" + strconv.Itoa(maskToCidr(n.Mask))
}

func NewIPNetSet() *IPNetSet {
	return &IPNetSet{
		inets:  make(map[IPNet]struct{}),
		single: make(map[IPNet]struct{}),
	}
}

type IPNetSet struct {
	inets  map[IPNet]struct{}
	single map[IPNet]struct{}
	ranges []*Range
}

// Add inserts an IPStr into the IPNetSet. If i has an elem that equals or contains the
// IPStr, nothing will happen. If i has more than one elem that is contained by the
// IPStr, insert into ordered slice and delete related elems.
//
// The first return value represents deleted IPNets. The second return value indicates
// whether an actual insertion occurs.
func (i *IPNetSet) Add(ip interface{}) ([]IPNet, bool) {
	var ipnet IPNet
	switch ip := ip.(type) {
	case string:
		ipnet, _ = Atoin(ip)
	case IPNet:
		ipnet = ip
	}

	if i.Empty() {
		i.inets[ipnet] = struct{}{}
		if ipnet.Mask != UINT32_MAX {
			r := IPNetToRange(ipnet)
			i.ranges = []*Range{r}
		} else {
			i.single[ipnet] = struct{}{}
		}
		return nil, true
	}

	if _, ok := i.inets[ipnet]; ok {
		return nil, false
	}

	var del = make([]IPNet, 0)
	var ok bool
	r := IPNetToRange(ipnet)

	// lookup /32 addrs.
	//
	// TODO: this is an ad-hoc solution. A better solution is to convert /32 addrs into
	// IRanges, and `search' should support this.
	for n := range i.single {
		if !r.contains(n.IPAddr) {
			continue
		}
		del = append(del, n)
		delete(i.inets, n)
		delete(i.single, n)
		ok = true
	}
	// lookup CIDRs
	idx, c := i.search(ipnet)
	switch c {
	case -1, 0:
	case -2:
		i.inets[ipnet] = struct{}{}
		// if ipnet is a single IP, no need to insert it to ranges
		if ipnet.Mask == UINT32_MAX {
			i.single[ipnet] = struct{}{}
			ok = true
			goto end
		}
		if idx == len(i.ranges) {
			i.ranges = append(i.ranges, r)
		} else {
			i.ranges = append(i.ranges[:idx+1], i.ranges[idx:]...)
			i.ranges[idx] = r
		}
		ok = true
	case 1:
		i.inets[ipnet] = struct{}{}
		end := idx
		for end < len(i.ranges) && bytes.Compare(i.ranges[end].end, r.end) <= 0 {
			for _, in := range i.ranges[end].ToIPNets() {
				n := IPNet{Ntoui(in.IP), Ntoui(in.Mask)}
				del = append(del, n)
				delete(i.inets, n)
			}
			end++
		}
		i.ranges[idx] = r
		// delete elements from idx+1 to end
		i.ranges = append(i.ranges[:idx+1], i.ranges[end:]...)
		ok = true
	default: // should not happen
	}

end:
	return del, ok
}

func (i *IPNetSet) Delete(inets ...IPNet) {
	for _, n := range inets {
		if _, ok := i.inets[n]; !ok {
			return
		}
		delete(i.inets, n)
		if n.Mask == UINT32_MAX {
			delete(i.single, n)
			continue // no elem in i.ranges
		}
		nr := IPNetToRange(n)
		idx := sort.Search(len(i.ranges), func(idx int) bool {
			return bytes.Compare(i.ranges[idx].start, nr.start) >= 0
		})
		i.ranges = append(i.ranges[:idx], i.ranges[idx+1:]...)
	}
}

// The first return value indicates the insert position. The second return value will be
// -2 if unrelated, -1 if n has parent in i, 0 if equal, 1 if n has children in i.
//
// Note that usually len(i.ranges) is not equal to i.Size(). The latter represents all
// IPNets including /32 CIDRs.
func (i *IPNetSet) search(n IPNet) (int, int) {
	if len(i.ranges) == 0 {
		return 0, -2
	}

	nr := IPNetToRange(n)
	idx := sort.Search(len(i.ranges), func(idx int) bool {
		return bytes.Compare(i.ranges[idx].start, nr.start) >= 0
	})

	if idx == len(i.ranges) { // no hit
		if bytes.Compare(nr.end, i.ranges[idx-1].end) <= 0 {
			return idx - 1, -1 // i.ranges[idx-1] contains nr
		} else {
			return idx, -2 // nr should be inserted at the rear
		}
	}

	if i.ranges[idx].start.Equal(nr.start) {
		// need to replace i.range[idx] with n
		// cases for bytes.Compare(nr.end, i.ranges[idx].end)
		// -1: nr is contained by i.range[idx]
		// 	  nr.start = idx.start <= nr.end < idx.end
		// 0: nr is equal to i.range[idx]
		// 	  nr.start = idx.start < nr.end = idx.end
		// 1: nr contains at least i.range[idx]
		//    nr.start = idx.start < idx.end < nr.end
		return idx, bytes.Compare(nr.end, i.ranges[idx].end)
	} else {
		if idx != 0 && bytes.Compare(nr.end, i.ranges[idx-1].end) <= 0 {
			return idx - 1, -1
		}
		if bytes.Compare(nr.end, i.ranges[idx].end) >= 0 {
			return idx, 1
		} else {
			return idx, -2
		}
	}
}

// Has means exact match. Elems that contain IP do not count.
func (i *IPNetSet) Has(ip interface{}) bool {
	var n IPNet
	switch ip := ip.(type) {
	case string:
		n, _ = Atoin(ip)
	case IPNet:
		n = ip
	}
	if _, ok := i.inets[n]; ok {
		return true
	}
	return false
}

// Contains means exact or CIDR-based match.
func (i *IPNetSet) Contains(ip interface{}) bool {
	var n IPNet
	switch ip := ip.(type) {
	case string:
		n, _ = Atoin(ip)
	case IPNet:
		n = ip
	}
	if _, ok := i.inets[n]; ok {
		return true
	} else {
		_, c := i.search(n)
		return c == 0 || c == -1
	}
}

func (i *IPNetSet) Values() map[IPNet]struct{} {
	return i.inets
}

func (i *IPNetSet) Size() int {
	return len(i.inets)
}

func (i *IPNetSet) Empty() bool {
	return len(i.inets) == 0
}

func (i *IPNetSet) ClearAll() {
	i.inets = make(map[IPNet]struct{})
	i.single = make(map[IPNet]struct{})
	i.ranges = nil
}

func (is *IPNetSet) ToList() []string {
	var ipList = make([]string, is.Size())
	var idx = 0
	for lr := range is.inets {
		ipList[idx] = lr.String()
		idx++
	}
	return ipList
}

func (is *IPNetSet) ToRuleSet() *RuleSet {
	rs := &RuleSet{rules: make(map[Match]Action, is.Size())}
	for ip := range is.inets {
		rs.rules[Match{Type: "l3", SrcIP: ip.String()}] = Action{Drop: ACTION_DROP}
	}
	return rs
}

func (is *IPNetSet) DumpFile(fname string) error {
	file, err := os.OpenFile(fname, os.O_CREATE|os.O_TRUNC|os.O_WRONLY, 0644)
	if err != nil {
		return err
	}
	defer file.Close()

	datawriter := bufio.NewWriter(file)
	for ip := range is.inets {
		_, _ = datawriter.WriteString(ip.String() + "\n")
	}
	return datawriter.Flush()
}

func (is *IPNetSet) MarshalJSON() ([]byte, error) {
	n := len(is.inets)
	if n == 0 {
		return []byte("null"), nil
	}
	var buf bytes.Buffer
	buf.Grow(10 * len(is.inets))
	buf.WriteByte('[')
	i := 0
	for inet := range is.inets {
		if i > 0 {
			buf.WriteByte(',')
		}
		fmt.Fprintf(&buf, "%q", inet)
		i++
	}
	buf.WriteByte(']')
	return buf.Bytes(), nil
}

func (is *IPNetSet) UnmarshalJSON(v []byte) error {
	var inets []string
	if err := json.Unmarshal(v, &inets); err != nil {
		return err
	}
	for _, inet := range inets {
		is.Add(inet)
	}
	return nil
}

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

// Atoui converts an IPv4 string to uint32
func Atoui(ipstr string) uint32 {
	ip := net.ParseIP(ipstr).To4()
	if ip == nil {
		return 0
	}
	return binary.BigEndian.Uint32(ip)
}

func Atoin(ipstr string) (IPNet, error) {
	var tmpIP IPNet
	_, ipnet, err := net.ParseCIDR(ipstr)
	if err == nil && ipnet != nil {
		tmpIP.IPAddr = binary.BigEndian.Uint32(ipnet.IP)
		tmpIP.Mask = binary.BigEndian.Uint32(ipnet.Mask)
	} else {
		ip := net.ParseIP(ipstr).To4()
		if ip == nil {
			return tmpIP, fmt.Errorf("invalid ip address: %v", ipstr)
		}
		tmpIP.IPAddr = binary.BigEndian.Uint32(ip)
		tmpIP.Mask = UINT32_MAX
	}
	return tmpIP, nil
}

// TODO
func ValidateIPNet(ip IPNet) bool {
	return true
}

func MergeIPNetSet(a *IPNetSet, b *IPNetSet, limit int) *IPNetSet {
	if b.Size() == 0 {
		return a
	}

	l := b.Size()

	if l > limit {
		//need to delete element
		panic(nil)
	}

	l = a.Size()
	if l > limit {
		//need to delete element
		for ip := range a.Values() {
			a.Delete(ip)
			if l -= 1; l == limit {
				break
			}
		}
	} else if l == limit {
		return a
	} else {
		//add element until to limit
		for ip := range b.Values() {
			// if a.Contains(ip) {
			// 	continue
			// }
			a.Add(ip)
			if l += 1; l == limit {
				break
			}
		}
	}
	return a
}
