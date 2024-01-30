package runner

import (
	flowmng "github.com/amianetworks/dni/src/flowmanager"
)

type PacketSamplingExecutor struct {
	ManageNetif    string
	ProtectedNetif []string
}

func (e *PacketSamplingExecutor) Start(value []flowmng.DataSpec) (flowmng.DataSpec, error) {
	// 开启采集
	return flowmng.DataSpec{}, nil
}

func (e *PacketSamplingExecutor) Stop() error {
	// 停止采集
	return nil
}
