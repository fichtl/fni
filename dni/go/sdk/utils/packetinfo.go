package utils

import (
	"fmt"

	"github.com/google/gopacket"
	"github.com/google/gopacket/layers"
)

type PacketInfo struct {
	SIP    uint32
	DIP    uint32
	SPort  uint32
	DPort  uint32
	Proto  uint32
	Length uint32
}

func (pinfo *PacketInfo) String() string {
	sip := Uitoa(pinfo.SIP)
	dip := Uitoa(pinfo.DIP)
	return fmt.Sprintf("SIP:%s DIP:%s SPort:%d DPort:%d Proto:%d Length:%d", sip, dip, pinfo.SPort, pinfo.DPort, pinfo.Proto, pinfo.Length)
}

func (pinfo *PacketInfo) ToMap() map[string]uint32 {
	pinfoMap := make(map[string]uint32)
	pinfoMap["SIP"] = pinfo.SIP
	pinfoMap["DIP"] = pinfo.DIP
	pinfoMap["SPort"] = pinfo.SPort
	pinfoMap["DPort"] = pinfo.DPort
	pinfoMap["Proto"] = pinfo.Proto
	pinfoMap["Length"] = pinfo.Length
	return pinfoMap
}

func GetPacketLength(packet gopacket.Packet) uint32 {
	return uint32(packet.Metadata().Length)
}

/*
Get packet information.
*/
func GetPacketInfo(packet gopacket.Packet) *PacketInfo {
	ethlayer := packet.Layer(layers.LayerTypeEthernet)
	if ethlayer == nil {
		fmt.Println("ethlayer error!")
		return nil
	}
	//If packet has TCP Layer
	tcplayer := packet.Layer(layers.LayerTypeTCP)
	if tcplayer != nil {
		pinfo := GetPacketInfoTCP(packet)
		return pinfo
	}
	//If packet has UDP Layer
	udplayer := packet.Layer(layers.LayerTypeUDP)
	if udplayer != nil {
		pinfo := GetPacketInfoUDP(packet)
		return pinfo
	}
	//If packet has ICMP Layer
	icmplayer := packet.Layer(layers.LayerTypeICMPv4)
	if icmplayer != nil {
		pinfo := GetPacketInfoICMP(packet)
		return pinfo
	}

	//ip
	iplayer := packet.Layer(layers.LayerTypeIPv4)
	if iplayer != nil {
		pinfo := GetPacketInfoIP(packet)
		return pinfo
	}
	return nil
}

/*
Get TCP packet information.
*/
func GetPacketInfoTCP(packet gopacket.Packet) *PacketInfo {
	iplayer := packet.Layer(layers.LayerTypeIPv4)
	if iplayer == nil {
		return nil
	}
	ip, ok := iplayer.(*layers.IPv4)
	if !ok {
		return nil
	}

	tcplayer := packet.Layer(layers.LayerTypeTCP)
	if tcplayer == nil {
		return nil
	}
	tcp, ok := tcplayer.(*layers.TCP)
	if !ok {
		return nil
	}
	pinfo := &PacketInfo{}
	pinfo.Proto = uint32(ip.Protocol)
	pinfo.SIP = Ntoui(ip.SrcIP.To4())
	pinfo.DIP = Ntoui(ip.DstIP.To4())
	pinfo.SPort = uint32(tcp.SrcPort)
	pinfo.DPort = uint32(tcp.DstPort)
	pinfo.Length = GetPacketLength(packet)
	//direction
	return pinfo
}

/*
Get UDP packet information.
*/
func GetPacketInfoUDP(packet gopacket.Packet) *PacketInfo {
	iplayer := packet.Layer(layers.LayerTypeIPv4)
	if iplayer == nil {
		return nil
	}
	ip, ok := iplayer.(*layers.IPv4)
	if !ok {
		return nil
	}

	udplayer := packet.Layer(layers.LayerTypeUDP)
	if udplayer == nil {
		return nil
	}
	udp, ok := udplayer.(*layers.UDP)
	if !ok {
		return nil
	}
	pinfo := &PacketInfo{}
	pinfo.Proto = uint32(ip.Protocol)
	pinfo.SIP = Ntoui(ip.SrcIP.To4())
	pinfo.DIP = Ntoui(ip.DstIP.To4())
	pinfo.SPort = uint32(udp.SrcPort)
	pinfo.DPort = uint32(udp.DstPort)
	pinfo.Length = GetPacketLength(packet)
	//direction
	return pinfo
}

/*
Get ICMP packet information.
*/
func GetPacketInfoICMP(packet gopacket.Packet) *PacketInfo {
	iplayer := packet.Layer(layers.LayerTypeIPv4)
	if iplayer == nil {
		return nil
	}
	ip, ok := iplayer.(*layers.IPv4)
	if !ok {
		return nil
	}

	icmplayer := packet.Layer(layers.LayerTypeICMPv4)
	if icmplayer == nil {
		return nil
	}
	pinfo := &PacketInfo{}
	pinfo.Proto = uint32(ip.Protocol)
	pinfo.SIP = Ntoui(ip.SrcIP.To4())
	pinfo.DIP = Ntoui(ip.DstIP.To4())
	pinfo.SPort = uint32(0)
	pinfo.DPort = uint32(0)
	pinfo.Length = GetPacketLength(packet)
	//direction
	return pinfo
}

/*
Get IP packet information.
*/
func GetPacketInfoIP(packet gopacket.Packet) *PacketInfo {
	iplayer := packet.Layer(layers.LayerTypeIPv4)
	if iplayer == nil {
		return nil
	}
	ip, ok := iplayer.(*layers.IPv4)
	if !ok {
		return nil
	}

	pinfo := &PacketInfo{}
	pinfo.Proto = uint32(ip.Protocol)
	pinfo.SIP = Ntoui(ip.SrcIP.To4())
	pinfo.DIP = Ntoui(ip.DstIP.To4())
	pinfo.SPort = uint32(0)
	pinfo.DPort = uint32(0)
	pinfo.Length = GetPacketLength(packet)
	return pinfo
}
