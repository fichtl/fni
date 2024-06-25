// TODO:
//
// 1. replacing packet decoders by `gopacket.DecodingLayerContainer', which is claimed to
// be faster.
// (https://pkg.go.dev/github.com/google/gopacket#hdr-Implementing_Your_Own_Decoder)
//
// 2. replacing IP/Port-based statistics by Flow-based statistics
package analyzer

import (
	"time"

	"github.com/amianetworks/dni/sdk/task/dms/common"

	"github.com/google/gopacket"
	"github.com/google/gopacket/ip4defrag"
	"github.com/google/gopacket/layers"
)

var (
	ipThreshold  = 8192
	tcpThreshold = 8192
	udpThreshold = 8192
	decodeOpt    = gopacket.DecodeOptions{
		Lazy:   true,
		NoCopy: true,
	}
)

type ProtoCounter struct {
	dev     string
	total   int
	filter  string
	snaplen int

	first     bool
	starttime time.Time

	fic, ft, fu, ful bool
	defrag           *ip4defrag.IPv4Defragmenter
	*rawProtoData    // runtime statistics
	*ProtoStat       // final results

	err error
}

func NewProtoCounter(dev string, protos ...string) *ProtoCounter {
	c := new(ProtoCounter)
	c.dev = dev
	c.first = true
	c.rawProtoData = newRawProtoData()
	c.ProtoStat = newProtoStat()
	c.snaplen = -1
protos:
	// select which proto is of intersting, if nothing specified, ProtoCounter will only
	// check IP headers
	for _, proto := range protos {
		switch proto {
		case "all":
			c.fic = true
			c.ft = true
			c.fu = true
			c.ful = true
			break protos
		case "icmp":
			c.fic = true
		case "tcp":
			c.ft = true
		case "udp":
			c.fu = true
		case "udplite":
			c.ful = true
		}
	}
	c.defrag = ip4defrag.NewIPv4Defragmenter()
	return c
}

func (c *ProtoCounter) Device() string {
	return c.dev
}

func (c *ProtoCounter) Filter() string {
	return c.filter
}

func (c *ProtoCounter) Snaplen() int {
	return c.snaplen
}

func (c *ProtoCounter) Count(packet gopacket.Packet) {
	if c.first {
		c.starttime = time.Now()
	}
	ipLayer := packet.Layer(layers.LayerTypeIPv4)
	if ipLayer == nil {
		c.total++
		return
	}

	var sip, dip string
	var meta pktMeta

	ip, ok := ipLayer.(*layers.IPv4)
	if !ok {
		c.total++
		return
	}
	//ip frag packet
	ip, err := c.defrag.DefragIPv4(ip)
	if err != nil {
		return
	}
	if ip == nil {
		return
	}
	//ip count
	c.IP.Count++
	sip = ip.SrcIP.String()
	dip = ip.DstIP.String()
	c.src[sip]++
	c.dst[dip]++
	//meta data
	meta.Lip = dip
	meta.Rip = sip
	meta.Len += len(ip.Contents)

	switch ip.Protocol {
	case layers.IPProtocolICMPv4:
		if c.fic {
			c.ICMP.Count++
			c.icmpsrc[sip]++
			c.icmpdst[dip]++
			countICMP(packet, c.ICMP)
		}
	case layers.IPProtocolTCP:
		if c.ft {
			c.TCP.Total++
			if len(ip.Payload) > 26 {
				c.IP.FragCount += 1
				packet = gopacket.NewPacket(ip.Payload, layers.LayerTypeTCP, decodeOpt)
			}
			countTCP(packet, &meta, c.tcp, c.TCP.Port, c.TCP.Flag)
		}
	case layers.IPProtocolUDP:
		if c.fu {
			c.UDP.Total++
			if len(ip.Payload) > 26 {
				c.IP.FragCount += 1
				packet = gopacket.NewPacket(ip.Payload, layers.LayerTypeUDP, decodeOpt)
			}
			countUDP(packet, &meta, c.udp, c.UDP.Port, c.UDP.SrcPort)
		}
	case layers.IPProtocolUDPLite:
		if c.ful {
			c.UDP.Total++
			if len(ip.Payload) > 26 {
				c.IP.FragCount += 1
				packet = gopacket.NewPacket(ip.Payload, layers.LayerTypeUDPLite, decodeOpt)
			}
			countUDPLite(packet, &meta, c.udp, c.UDP.Port, c.UDP.SrcPort)
		}
	}

	c.total++
}

func (c *ProtoCounter) ShouldStop() bool {
	return c.total >= 10000
}

func (c *ProtoCounter) Summarize() {
	c.raw = c.rawProtoData

	c.IP.SIP = c.src.SortByCount(-1)
	c.IP.DIP = c.dst.SortByCount(-1)
	if c.ft {
		c.TCP.NSrcPeer = len(c.tcp.srcpeer) //tcp src+sport
		c.TCP.NDstPeer = len(c.tcp.dstpeer)
		c.TCP.NFlow = len(c.tcp.flows)
	}
	if c.fu {
		c.UDP.NSrcPeer = len(c.udp.srcpeer)
		c.UDP.NDstPeer = len(c.udp.dstpeer)
		c.UDP.NFlow = len(c.udp.flows)
	}
}

func (c *ProtoCounter) Finish() {
	c.defrag = nil
}

func (c *ProtoCounter) Error(err error) {
	c.err = err
	c.Finish()
}

// iptotallen-ipheaderlen
const (
	PKT_LEN_LESS_THAN_100 uint16 = iota
	PKT_LEN_BETWEEN_100_AND_1500
	PKT_LEN_MORE_THAN_1500
)

func countIPFragLen(payloadLen uint16, lenStat map[uint16]int) {
	if payloadLen <= 100 {
		lenStat[PKT_LEN_LESS_THAN_100] += 1
	} else if payloadLen > 100 && payloadLen <= 1500 {
		lenStat[PKT_LEN_BETWEEN_100_AND_1500] += 1
	} else {
		lenStat[PKT_LEN_MORE_THAN_1500] += 1
	}
}

func countICMP(packet gopacket.Packet, stat ICMPStat) {
	icmpLayer := packet.Layer(layers.LayerTypeICMPv4)
	if icmpLayer == nil {
		return
	}

	icmp, _ := icmpLayer.(*layers.ICMPv4)
	stat.Type[icmp.TypeCode.Type()] += 1
}

// TCP protocol statistics according to flags or port
func countTCP(packet gopacket.Packet, meta *pktMeta, stat *transStatData, dstStat map[common.PORT]int, flagStat map[uint8]int) {
	tcpLayer := packet.Layer(layers.LayerTypeTCP)
	if tcpLayer == nil {
		return
	}
	tcp, ok := tcpLayer.(*layers.TCP)
	if !ok || len(tcp.Contents) < common.TCP_HEADER_MIN_LEN { // unlikely
		return
	}

	meta.Rport = common.PORT(tcp.SrcPort)
	meta.Lport = common.PORT(tcp.DstPort)
	meta.Len += (len(tcp.Contents) + len(tcp.Payload))

	stat.src[common.PORT(tcp.SrcPort)] += 1
	stat.dst[common.PORT(tcp.DstPort)] += 1
	stat.flows[meta.Flow] += 1

	// if _, ok := stat.pip[common.PORT(tcp.DstPort)]; !ok {
	// 	stat.pip[common.PORT(tcp.DstPort)] = make(common.IPStatMap)
	// }
	// stat.pip[common.PORT(tcp.DstPort)][meta.Rip] += 1

	peer := common.IPP{meta.Rip, meta.Rport}
	stat.srcpeer[peer] += 1
	if _, ok := stat.pp[common.PORT(tcp.DstPort)]; !ok {
		stat.pp[common.PORT(tcp.DstPort)] = make(common.IPPStatMap)
	}
	stat.pp[common.PORT(tcp.DstPort)][peer] += 1

	peer = common.IPP{meta.Lip, meta.Lport}
	stat.dstpeer[peer] += 1

	if dstStat != nil {
		dstStat[common.PORT(tcp.DstPort)] += 1
	}

	if flagStat != nil {
		flag := tcp.Contents[13]
		flagStat[flag] += 1
		// group pktMetas by flag
		// if _, ok := stat.flag[flag]; !ok {
		// 	stat.flag[flag] = make([]*pktMeta, 0)
		// }
		stat.flag[flag] = append(stat.flag[flag], meta)
	}

	// if l7Stat != nil {
	// 	if ret := countTCPService(packet, tcp.DstPort, l7Stat[SrvType(tcp.DstPort)]); ret != 0 {
	// 		countTCPService(packet, tcp.SrcPort, l7Stat[SrvType(tcp.SrcPort)])
	// 	}
	// }
}

// UDP protocol statistics according to port
func countUDP(packet gopacket.Packet, meta *pktMeta, stat *transStatData, dstStat map[common.PORT]int, srcStat map[common.PORT]int) {
	udpLayer := packet.Layer(layers.LayerTypeUDP)
	if udpLayer == nil {
		return
	}
	udp, ok := udpLayer.(*layers.UDP)
	if !ok {
		return
	}

	meta.Rport = common.PORT(udp.SrcPort)
	meta.Lport = common.PORT(udp.DstPort)
	meta.Len += (len(udp.Contents) + len(udp.Payload))

	stat.src[common.PORT(udp.SrcPort)] += 1
	stat.dst[common.PORT(udp.DstPort)] += 1
	stat.flows[meta.Flow] += 1

	peer := common.IPP{meta.Rip, meta.Rport}
	stat.srcpeer[peer] += 1
	if _, ok := stat.pp[common.PORT(udp.DstPort)]; !ok {
		stat.pp[common.PORT(udp.DstPort)] = make(common.IPPStatMap)
	}
	stat.pp[common.PORT(udp.DstPort)][peer] += 1

	stat.dstpeer[common.IPP{meta.Lip, meta.Lport}] += 1

	if dstStat != nil {
		dstStat[common.PORT(udp.DstPort)] += 1
	}

	if srcStat != nil {
		srcStat[common.PORT(udp.SrcPort)] += len(udp.Payload)
	}

	stat.metas = append(stat.metas, meta)

	// Counting UDP-based application protocols. There is no efficient method to
	// distinguish application layer type, so we only use ports to specify its type. It
	// is necessary to check the src port for checking reflection attacks.
	// if l7Stat != nil {
	// 	if ret := countUDPService(packet, udp.DstPort, l7Stat[SrvType(udp.DstPort)]); ret != 0 {
	// 		countUDPService(packet, udp.SrcPort, l7Stat[SrvType(udp.SrcPort)])
	// 	}
	// }
}

func countUDPLite(packet gopacket.Packet, meta *pktMeta, stat *transStatData, dstStat map[common.PORT]int, srcStat map[common.PORT]int) {
	udpLayer := packet.Layer(layers.LayerTypeUDPLite)
	if udpLayer == nil {
		return
	}

	udp, ok := udpLayer.(*layers.UDPLite)
	if !ok {
		return
	}

	meta.Rport = common.PORT(udp.SrcPort)
	meta.Lport = common.PORT(udp.DstPort)
	meta.Len += (len(udp.Contents) + len(udp.Payload))

	stat.src[common.PORT(udp.SrcPort)] += 1
	stat.dst[common.PORT(udp.DstPort)] += 1
	stat.flows[meta.Flow] += 1

	peer := common.IPP{meta.Rip, meta.Rport}
	stat.srcpeer[peer] += 1
	if _, ok := stat.pp[common.PORT(udp.DstPort)]; !ok {
		stat.pp[common.PORT(udp.DstPort)] = make(common.IPPStatMap)
	}
	stat.pp[common.PORT(udp.DstPort)][peer] += 1

	stat.dstpeer[common.IPP{meta.Lip, meta.Lport}] += 1

	if dstStat != nil {
		dstStat[common.PORT(udp.DstPort)] += 1
	}

	if srcStat != nil {
		srcStat[common.PORT(udp.SrcPort)] += len(udp.Payload)
	}

	stat.metas = append(stat.metas, meta)
}
