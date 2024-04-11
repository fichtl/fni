package runner

import (
	"fmt"
	"log"
	"time"

	flowmng "github.com/amianetworks/dni/sdk/flowmanager"
	"github.com/google/gopacket"
	"github.com/google/gopacket/pcap"
)

type PacketCountFromPCAPExecutor struct {
	RunnerName string
	outputNum  int
}

func NewPacketCountFromPCAPExecutor(runner string, options map[string]interface{}) Executor {
	return &PacketCountFromPCAPExecutor{
		RunnerName: runner,
		outputNum:  1,
	}
}

func (e *PacketCountFromPCAPExecutor) Start(value []*flowmng.DataSpec) ([]flowmng.DataSpec, error) {
	outputs := make([]flowmng.DataSpec, 0)
	if len(value) == 0 {
		return []flowmng.DataSpec{}, fmt.Errorf("packet count runner: input num error")
	}
	for _, v := range value {
		sum := 0
		handle, ok := v.Data.(*pcap.Handle)
		if !ok {
			return []flowmng.DataSpec{}, fmt.Errorf("packet count runner cast error")
		}
		packetSource := gopacket.NewPacketSource(handle, handle.LinkType())
		for range packetSource.Packets() {
			sum++
		}
		d := flowmng.DataSpec{
			Data:      sum,
			TimeStamp: time.Now(),
			Type:      flowmng.DATA_TYPE_INT,
		}
		log.Printf("packet count:%d", sum)
		outputs = append(outputs, d)
	}
	return outputs, nil
}

func (e *PacketCountFromPCAPExecutor) Stop() error {
	return nil
}
