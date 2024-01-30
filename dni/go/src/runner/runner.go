package runner

import (
	"fmt"

	flowmng "github.com/amianetworks/dni/src/flowmanager"
)

const (
	PACKET_SAMPLING_RUNNER        string = "PacketSamplingRunner"
	PACKET_COUNT_FROM_PCAP_RUNNER string = "PacketCountFromPCAPRunner"
	INT_SUM_RUNNER                string = "IntSumRunner"
)

type Executor interface {
	Start(value []flowmng.DataSpec) (flowmng.DataSpec, error)
	Stop() error
}

func NewRunnerExecutor(runner string) (Executor, error) {
	switch runner {
	case INT_SUM_RUNNER:
		return NewIntSumExecutor(), nil
	case PACKET_COUNT_FROM_PCAP_RUNNER:
		return NewPacketCountFromPCAPExecutor(), nil
	default:
		return nil, fmt.Errorf("unsupported runner: %s", runner)
	}
}
