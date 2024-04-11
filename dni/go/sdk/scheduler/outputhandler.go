package scheduler

import (
	flowmng "github.com/amianetworks/dni/sdk/flowmanager"
)

type OutputStreamHandler interface {
	PrepareForRun() error
	GetOutputChannel(string) (chan flowmng.DataSpec, bool)
	GetInputManager() *flowmng.InputManager
	Close() error
}

type GraphOutputHandler struct {
	InputManager *flowmng.InputManager
}

func NewGraphOutputHandler(gOutputStreams []string) *GraphOutputHandler {
	h := &GraphOutputHandler{
		InputManager: flowmng.NewInputManager(gOutputStreams),
	}
	return h
}

func (h *GraphOutputHandler) PrepareForRun() error {
	return nil
}

func (h *GraphOutputHandler) GetInputManager() *flowmng.InputManager {
	return h.InputManager
}

func (h *GraphOutputHandler) GetOutputChannel(stream string) (chan flowmng.DataSpec, bool) {
	outCh, ok := h.InputManager.GetInputStreamChannel(stream)
	return outCh, ok
}

func (h *GraphOutputHandler) Close() error {
	return nil
}
