package analyzer

import (
	"github.com/amianetworks/dni/sdk/task/dms/common"
)

type ProtoDiag struct {
	Tcp, Udp *common.PortInfo
	Stat     map[string]*ProtoStat
}

func NewProtoDiag() *ProtoDiag {
	return &ProtoDiag{
		Tcp:  common.NewPortInfo(),
		Udp:  common.NewPortInfo(),
		Stat: make(map[string]*ProtoStat),
	}
}

type ProtoStat struct { // results of proto analyser
	IP    IPStat //IP状态
	ICMP  ICMPStat
	TCP   TCPStat
	UDP   UDPStat
	raw   *rawProtoData
	Ratio float64
}

func newProtoStat() *ProtoStat {
	return &ProtoStat{
		IP:   IPStat{0, 0, make(map[uint16]int), nil, nil},
		ICMP: ICMPStat{0, make(map[uint8]int), nil, nil},
		TCP:  TCPStat{Total: 0, Flag: make(map[uint8]int)},
		UDP:  UDPStat{Total: 0, SrcPort: make(map[common.PORT]int)},
	}
}

type IPStat struct {
	Count     int
	FragCount int
	FragLen   map[uint16]int
	SIP, DIP  []common.IPCount
}

type ICMPStat struct {
	Count    int
	Type     map[uint8]int //icmp类型-包个数
	SIP, DIP []common.IPCount
}

type TCPStat struct {
	Total    int
	NSrcPeer int
	NDstPeer int
	NFlow    int
	Port     map[common.PORT]int
	Flag     map[uint8]int
}

type UDPStat struct {
	Total    int
	NSrcPeer int
	NDstPeer int
	NFlow    int
	Port     map[common.PORT]int
	SrcPort  map[common.PORT]int
}

// Per-state connection statistics
type ConnStat struct {
	Count   int
	Flows   []common.FlowStat
	PerPort common.PortConnInfo
}

type rawProtoData struct {
	*ipStatData
	tcp, udp *transStatData
}

func newRawProtoData() *rawProtoData {
	return &rawProtoData{
		ipStatData: newIPStatData(),
		tcp:        newTransStatData(),
		udp:        newTransStatData(),
	}
}

type ipStatData struct {
	// Total statistics
	src, dst common.IPCountMap
	// Per-protocol statistics of remote IP addresses
	icmpsrc, icmpdst common.IPCountMap
}

func newIPStatData() *ipStatData {
	return &ipStatData{
		make(common.IPCountMap), make(common.IPCountMap), // IP
		make(common.IPCountMap), make(common.IPCountMap), // ICMP
	}
}

type transStatData struct {
	// Src/Dst port statistics
	src, dst common.PortCountMap
	// Src/Dst IPP statistics
	srcpeer, dstpeer common.IPPStatMap
	// Flow statistics, to replace src/dst port statistics
	flows map[common.Flow]int

	pp map[common.PORT]common.IPPStatMap // Per-dstport statistics of remote peer.

	// Per-flag statistics of remote IP addresses, this is only valid for TCP traffics
	flag  map[uint8][]*pktMeta // TCP only,per-flag statistics
	metas []*pktMeta           // UDP only, per-srcport statistics of remote IP addresses.
}

func newTransStatData() *transStatData {
	return &transStatData{
		make(common.PortCountMap), make(common.PortCountMap),
		make(common.IPPStatMap), make(common.IPPStatMap),
		make(map[common.Flow]int),
		make(map[common.PORT]common.IPPStatMap),
		make(map[uint8][]*pktMeta),
		make([]*pktMeta, 0),
	}
}

type pktMeta struct {
	common.Flow
	Len  int
	Flag uint8
}
