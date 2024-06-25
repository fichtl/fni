package common

import (
	"fmt"
	"sort"
)

type Peer interface {
	Host() string
	Port() PORT
}

type IP string

func (i IP) Host() string   { return string(i) }
func (i IP) Port() PORT     { return 0 }
func (i IP) String() string { return i.Host() }

type IPP struct {
	IP string
	PORT
}

func (i IPP) Host() string   { return i.IP }
func (i IPP) Port() PORT     { return i.PORT }
func (i IPP) String() string { return fmt.Sprintf("%s:%d", i.IP, i.PORT) }

type IPSlice []uint32

func (a IPSlice) Len() int           { return len(a) }
func (a IPSlice) Less(i, j int) bool { return a[i] < a[j] }
func (a IPSlice) Swap(i, j int)      { a[i], a[j] = a[j], a[i] }

type PeerStat interface {
	Peer() Peer
	GetCount() uint64
}

type IPCount struct {
	IP    string
	Count uint64
}

func (c *IPCount) GetCount() uint64 { return c.Count }

// []IPCount, []IPPStat
type Statistics interface {
	Len() int
	Sort()
	Top(int, bool) Statistics
}

type IPCountMap map[string]uint64

func (s IPCountMap) SortByCount(n int) []IPCount {
	return SortIPCountMapByCount(s, n)
}

func (s IPCountMap) Sort(n int) []IPCount {
	return SortIPCountMapByCount(s, n)
}

// FindIP returns the index of an IP address in the slice; returns -1 if it's not found
func FindIP(ss []IPCount, val string) (int, bool) {
	for i, item := range ss {
		if item.IP == val {
			return i, true
		}
	}
	return -1, false
}

type byIPCount []IPCount

func (a byIPCount) Len() int           { return len(a) }
func (a byIPCount) Less(i, j int) bool { return a[i].Count > a[j].Count }
func (a byIPCount) Swap(i, j int)      { a[i], a[j] = a[j], a[i] }

// SortIPCountMapByCount find `IPCount's with top-n count.
//
// TODO: implement top-k sort instead of using built-in sort algorithm.
func SortIPCountMapByCount(m IPCountMap, n int) []IPCount {
	var s = make(byIPCount, len(m))
	i := 0
	for k, v := range m {
		s[i] = IPCount{k, v}
		i++
	}
	sort.Sort(s)
	if n < 0 || len(m) < n {
		n = len(m)
	}
	return s[:n]
}

type UIPCount struct {
	IP    uint32
	A     string
	Count uint64
}

type byIPSrc []UIPCount

func (a byIPSrc) Len() int           { return len(a) }
func (a byIPSrc) Less(i, j int) bool { return a[i].IP < a[j].IP }
func (a byIPSrc) Swap(i, j int)      { a[i], a[j] = a[j], a[i] }

func SortIPCountMapBySrc(m IPCountMap) []UIPCount {
	var s = make(byIPSrc, len(m))
	i := 0
	for k, v := range m {
		s[i] = UIPCount{Atoui(k), k, v}
		i++
	}
	sort.Sort(s)
	return s
}

type PortCount struct {
	Port  PORT
	Count uint64
}

func (s *PortCount) GetCount() uint64 { return s.Count }

type byPortCount []PortCount

func (a byPortCount) Len() int           { return len(a) }
func (a byPortCount) Less(i, j int) bool { return a[i].Count > a[j].Count }
func (a byPortCount) Swap(i, j int)      { a[i], a[j] = a[j], a[i] }

// Find a string in slice; only return index or bool=-1
func FindPort(ss []PortCount, val PORT) (int, bool) {
	for i, item := range ss {
		if item.Port == val {
			return i, true
		}
	}
	return -1, false
}

type PortCountMap map[PORT]uint64

func (s PortCountMap) Sort() []PortCount {
	return SortPortCountMap(s)
}

func SortPortCountMap(m PortCountMap) []PortCount {
	var s = make(byPortCount, len(m))
	i := 0
	for k, v := range m {
		s[i] = PortCount{k, v}
		i++
	}
	sort.Sort(s)
	return s
}

type IPPStat struct {
	IPP
	Count uint64
}

func (s *IPPStat) GetCount() uint64 { return s.Count }

type IPPStatMap map[IPP]uint64

type FlowSet map[Flow]struct{}

// TODO: rename
type FlowStat struct {
	Key   interface{}
	Count int
}

type FlowStatMap map[interface{}]FlowSet

type byFlowCount []FlowStat

func (a byFlowCount) Len() int           { return len(a) }
func (a byFlowCount) Less(i, j int) bool { return a[i].Count > a[j].Count }
func (a byFlowCount) Swap(i, j int)      { a[i], a[j] = a[j], a[i] }

func SortFlowStatMap(m FlowStatMap) []FlowStat {
	var s = make(byFlowCount, len(m))
	i := 0
	for k, v := range m {
		s[i] = FlowStat{k, len(v)}
		i++
	}
	sort.Sort(s)
	return s
}

type PortConnInfo map[PORT][]*ConnInfo

// A generic statistics for application layer protocols. Every protocol defined under
// common/services folder should map its request/respond types to a uint16 key. For
// example, GET/HEAD/POST methods in HTTP protocol will be translate to 1/2/3.
type SrvStatMap map[uint16]uint32
