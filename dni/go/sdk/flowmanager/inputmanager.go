package flowmanager

import (
	"fmt"
)

type InputManager struct {
	InputStreams   []string
	InputStreamMap map[string]int
	InputChannels  map[string]chan DataSpec
}

func NewInputManager(inputstream []string) *InputManager {
	//different input stream to different channel
	inputchannels := make(map[string]chan DataSpec)
	instreammap := make(map[string]int, 0)
	for id, stream := range inputstream {
		instreammap[stream] = id
		inputch := make(chan DataSpec, 1000)
		inputchannels[stream] = inputch
	}

	return &InputManager{
		InputStreams:   inputstream,
		InputStreamMap: instreammap,
		InputChannels:  inputchannels,
	}
}

func (im *InputManager) HasInputStream(stream string) bool {
	_, ok := im.InputStreamMap[stream]
	return ok
}

func (im *InputManager) GetInputStreamChannel(stream string) (chan DataSpec, bool) {
	inputch, ok := im.InputChannels[stream]
	return inputch, ok
}

func (im *InputManager) Subscribe() ([]*DataSpec, bool) {
	//return when all input stream data has been recieved
	var allstop bool = true
	inputdatas := make([]*DataSpec, len(im.InputStreams))
	for id, stream := range im.InputStreams {
		ch, _ := im.GetInputStreamChannel(stream)
		inputdata, ok := <-ch
		allstop = allstop && !ok
		inputdatas[id] = &inputdata
	}
	return inputdatas, allstop
}

func (im *InputManager) AddData(input DataSpec, stream string) error {
	ch, ok := im.GetInputStreamChannel(stream)
	if !ok {
		return fmt.Errorf("add data error: stream name error:%s", stream)
	}
	ch <- input
	return nil
}

func (im *InputManager) CloseInputChannel(stream string) error {
	ch, ok := im.GetInputStreamChannel(stream)
	if !ok {
		return fmt.Errorf("close channel error: stream (%s) channel not exist", stream)
	}
	close(ch)
	for range ch {
	}
	return nil
}
