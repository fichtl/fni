package runner

import (
	"time"

	"github.com/amianetworks/am.modules/log"
	flowmng "github.com/amianetworks/dni/src/flowmanager"
	"github.com/google/gopacket"
	"github.com/google/gopacket/pcap"
)

type PacketCountFromPCAPExecutor struct {
	LastOutput int
}

func NewPacketCountFromPCAPExecutor() *PacketCountFromPCAPExecutor {
	return &PacketCountFromPCAPExecutor{}
}

func (e *PacketCountFromPCAPExecutor) Start(value []flowmng.DataSpec) (flowmng.DataSpec, error) {
	data := flowmng.DataSpec{
		Type: flowmng.DATA_TYPE_INT,
	}
	if len(value) == 0 {
		data.Data = 0
		data.TimeStamp = time.Now()
		return data, nil
	}
	sum := 0
	for _, v := range value {
		switch v.Type {
		case flowmng.DATA_TYPE_PCAP_HANDLE:
			handle := v.Data.(*pcap.Handle)
			packetSource := gopacket.NewPacketSource(handle, handle.LinkType())
			for range packetSource.Packets() {
				sum++
			}
		}
	}
	data.Data = sum
	data.TimeStamp = time.Now()
	log.R.Debug("packet count: ", sum)
	return data, nil
}

func (e *PacketCountFromPCAPExecutor) Stop() error {
	return nil
}
