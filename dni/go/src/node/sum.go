package node

import (
	"fmt"
	"os"
	"time"

	"github.com/amianetworks/dni/src/data"
	"github.com/amianetworks/dni/src/design"
	"github.com/amianetworks/dni/src/graph"
)

type DataSumInputHandler struct {
	From string
	// 其他来源
}

type DataSumOutputHandler struct {
	To   string
	File design.FileConfig
}

// 初始化 input 和 output 的 handler
func InitializeDataSumInputHandler(nc graph.NodeConfig) DataSumInputHandler {
	return DataSumInputHandler{
		From: nc.InputStream.From,
	}
}

func InitializeDataSumOutputHandler(nc graph.NodeConfig) DataSumOutputHandler {
	return DataSumOutputHandler{
		To:   nc.OutputStream.To,
		File: nc.OutputStream.File,
	}
}

// input handler 的对应接口
func (dsih *DataSumInputHandler) PrepareForRun() error {
	return nil
}

func (dsih *DataSumInputHandler) ReceiveInputStreams(inputChan *data.InputManager) error {
	// 因为从其他节点来，所以不需要自己生成数据
	return nil
}

func (dsih *DataSumInputHandler) Close() error {
	return nil
}

// output handler 的对应接口
func (dsoh *DataSumOutputHandler) PrepareForRun() error {
	filehandle, err := os.OpenFile(dsoh.File.FileName, os.O_CREATE|os.O_RDWR|os.O_APPEND, 0664)
	if err != nil {
		return err
	}
	dsoh.File.FileHandle = filehandle
	return nil
}

func (dsoh *DataSumOutputHandler) SendOutputStreams(data data.DataSpec) error {
	realdata := data.Data.(int)
	dsoh.File.FileHandle.Truncate(0)
	if _, err := dsoh.File.FileHandle.WriteString(fmt.Sprint(realdata)); err != nil {
		return err
	}
	return nil
}

func (dsoh *DataSumOutputHandler) Close() error {
	switch dsoh.To {
	case design.OUTPUT_TO_FILE:
		dsoh.File.FileHandle.Close()
	}
	return nil
}

// 任务执行接口
func DataSumStart(dataArr []data.DataSpec, option data.DataSpec) (data.DataSpec, error) {
	if dataArr == nil {
		return data.DataSpec{}, nil
	}
	ldata := 0
	if option.Type == data.DATA_TYPE_INT {
		ldata = option.Data.(int)
	}

	for _, input := range dataArr {
		switch input.Type {
		case data.DATA_TYPE_INT:
			realdata := input.Data.(int)
			ldata += realdata
		case data.DATA_TYPE_INT_SLICE:
			realdata := input.Data.([]int)
			for _, d := range realdata {
				ldata += d
			}
		}
	}

	output := data.DataSpec{
		Type:      data.DATA_TYPE_INT,
		TimeStamp: time.Now(),
		Data:      ldata,
	}
	return output, nil
}

func DataSumStop(data data.DataSpec) error {
	return nil
}
