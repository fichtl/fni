package node

import (
	"github.com/amianetworks/am.modules/log"
	"github.com/amianetworks/dni/src/data"
	"github.com/amianetworks/dni/src/design"
	"github.com/amianetworks/dni/src/graph"
	"github.com/google/gopacket"
	"github.com/google/gopacket/pcap"
)

type DataStatisticsInputHandler struct {
	From     string
	PcapFile design.PcapFileConfig
	DB       design.DataBaseConfig
}

type DataStatisticsOutputHandler struct {
	To string
	DB design.DataBaseConfig
}

// input and output 初始化接口
func InitailizeDataStatisticsInputHandler(nc graph.NodeConfig) DataStatisticsInputHandler {
	inputHandler := DataStatisticsInputHandler{
		From: nc.InputStream.From,
	}
	switch inputHandler.From {
	case design.INPUT_FROM_PCAP_FILE:
		inputHandler.PcapFile = nc.InputStream.PcapFile
	case design.INPUT_FROM_DATABASE:
		inputHandler.DB = nc.InputStream.DataBase
	}
	return inputHandler
}

func InitailizeDataStatisticsOutputHandler(nc graph.NodeConfig) DataStatisticsOutputHandler {
	outputHandler := DataStatisticsOutputHandler{
		To: nc.OutputStream.To,
	}
	switch outputHandler.To {
	case design.OUTPUT_TO_DATABASE:
		outputHandler.DB = nc.OutputStream.DataBase
	}
	return outputHandler
}

// input handler 的对应接口
func (dsih *DataStatisticsInputHandler) PrepareForRun() error {
	return nil
}

func (dsih *DataStatisticsInputHandler) ReceiveInputStreams(inputChan *data.InputManager) error {
	input := data.DataSpec{}

	switch dsih.From {
	case design.INPUT_FROM_PCAP_FILE:
		// NOTE: handle需要close
		handle, err := pcap.OpenOffline(dsih.PcapFile.PcapFile)
		if err != nil {
			return err
		}
		input.Data = handle
		input.Type = data.DATA_TYPE_PCAP_HANDLE
	}
	inputChan.AddPacket(input)
	return nil
}

func (dsih *DataStatisticsInputHandler) Close() error {
	return nil
}

// output handler 的对应接口
func (dsoh *DataStatisticsOutputHandler) PrepareForRun() error {
	return nil
}

func (dsoh *DataStatisticsOutputHandler) SendOutputStreams(data data.DataSpec) error {
	return nil
}

func (dsoh *DataStatisticsOutputHandler) Close() error {
	switch dsoh.To {
	case design.OUTPUT_TO_DATABASE:
		// 关闭数据库的client
	}
	return nil
}

func DataStatisticsStart(dataArr []data.DataSpec, option data.DataSpec) (data.DataSpec, error) {
	if dataArr == nil {
		return data.DataSpec{}, nil
	}

	total := make([]int, 0)
	for _, input := range dataArr {
		count := 0
		switch input.Type {
		case data.DATA_TYPE_PCAP_HANDLE:
			handle := input.Data.(*pcap.Handle)
			packetSource := gopacket.NewPacketSource(handle, handle.LinkType())
			for range packetSource.Packets() {
				count++
			}
			handle.Close()
		}
		total = append(total, count)
	}
	output := data.DataSpec{
		Type: data.DATA_TYPE_INT_SLICE,
		Data: total,
	}
	log.R.Debug("total: ", total)
	return output, nil
}

func DataStatisticsStop(data data.DataSpec) error {
	// 最小的计数协程handle怎么处理
	return nil
}
