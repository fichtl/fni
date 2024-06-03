package test

import (
	"log"
	"testing"

	"github.com/amianetworks/dni/sdk/utils"
)

func TestGetPacketInfo(t *testing.T) {
	fin := "/home/yf/workspace/pcap/a10-10.pcap"
	utils.GetPacketInfos(fin)
}

func TestRBParse(t *testing.T) {
	rb, err := utils.NewShareMemomry(1, 100*utils.SlotSize)
	if err != nil {
		log.Printf("cretae shm failed:%v", err)
	}
	dataptr, ok := rb.Read()
	if ok {
		utils.RingBufferDataParse(dataptr)
	}
}
