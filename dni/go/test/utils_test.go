package test

import (
	"testing"

	"github.com/amianetworks/dni/sdk/utils"
)

func TestGetPacketInfo(t *testing.T) {
	fin := "/home/yf/workspace/pcap/a10-10.pcap"
	utils.GetPacketInfos(fin)
}
