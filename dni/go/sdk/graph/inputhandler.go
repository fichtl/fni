package graph

import (
	"fmt"

	config "github.com/amianetworks/dni/sdk/config"
	flowmng "github.com/amianetworks/dni/sdk/flowmanager"
)

type InputStreamHandler interface {
	PrepareForRun() error
	Close() error
	GetGraphInputStreams() []string
	AddGraphInputData(*flowmng.DataSpec, string) error
	SetSourceEdge(map[string][]*flowmng.InputManager)
}

type GraphInputHandler struct {
	OutputManager *flowmng.OutputManager
}

func NewGraphInputHandler(gInputStreams config.StreamUnit) *GraphInputHandler {
	h := &GraphInputHandler{
		OutputManager: flowmng.NewOutputManager(gInputStreams),
	}
	return h
}

func (h *GraphInputHandler) PrepareForRun() error {
	return nil
}

func (h *GraphInputHandler) SetSourceEdge(nextInputManagers map[string][]*flowmng.InputManager) {
	h.OutputManager.NextInputManagers = nextInputManagers
}

func (h *GraphInputHandler) AddGraphInputData(data *flowmng.DataSpec, stream string) error {
	err := h.OutputManager.AddData(data, stream)
	if err != nil {
		return fmt.Errorf("graph input handler error:%v", err)
	}
	return nil
}

func (h *GraphInputHandler) GetGraphInputStreams() []string {
	return h.OutputManager.OutputStreams
}

func (h *GraphInputHandler) Close() error {
	h.OutputManager.Close()
	return nil
}
