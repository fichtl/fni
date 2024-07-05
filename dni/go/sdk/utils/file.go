package utils

import (
	"github.com/google/gopacket"
	"github.com/google/gopacket/pcap"
)

func GetPacketInfos(fin string) (pinfos []map[string]uint32, err error) {
	//create packet source
	handle, err := pcap.OpenOffline(fin)
	if err != nil {
		return nil, err
	}
	pinfos = make([]map[string]uint32, 0)
	packetSource := gopacket.NewPacketSource(handle, handle.LinkType())
	for p := range packetSource.Packets() {
		pinfo := GetPacketInfo(p)
		if pinfo != nil {
			pinfoMap := pinfo.ToMap()
			pinfos = append(pinfos, pinfoMap)
		}
	}
	// fmt.Println(pinfos)
	return pinfos, nil
}
