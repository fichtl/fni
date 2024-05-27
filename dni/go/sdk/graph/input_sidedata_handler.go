package graph

import (
	"fmt"

	config "github.com/amianetworks/dni/sdk/config"
	flowmng "github.com/amianetworks/dni/sdk/flowmanager"
)

type GraphInputSideDataHandler struct {
	OutputSideDataManager *flowmng.OutputSideDataManager
}

func NewGraphInputSideDataHandler(gInputSideData config.StreamUnit) *GraphInputSideDataHandler {
	h := &GraphInputSideDataHandler{
		OutputSideDataManager: flowmng.NewOutputSideDataManager(gInputSideData),
	}
	return h
}

func (h *GraphInputSideDataHandler) PrepareForRun() error {
	return nil
}

func (h *GraphInputSideDataHandler) SetSourceEdge(nextInputSideData map[string][]*flowmng.DataSlice) {
	h.OutputSideDataManager.NextInputSideData = nextInputSideData
}

func (h *GraphInputSideDataHandler) AddGraphInputSideData(data *flowmng.DataSpec, sidedata string) error {
	err := h.OutputSideDataManager.AddData(data, sidedata)
	if err != nil {
		return fmt.Errorf("graph input handler error:%v", err)
	}
	return nil
}

func (h *GraphInputSideDataHandler) GetGraphInputSideData() []string {
	return h.OutputSideDataManager.OutputSidedata
}

func (h *GraphInputSideDataHandler) Close() error {
	err := h.OutputSideDataManager.Close()
	return err
}
