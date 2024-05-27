package flowmanager

import (
	"fmt"

	config "github.com/amianetworks/dni/sdk/config"
)

type InputManager struct {
	InputStreams  []string
	InputChannels map[string]chan *DataSpec
	Inputs        *DataSlice
}

func NewInputManager(inputstream config.StreamUnit) *InputManager {
	//different input stream to different channel
	inputchannels := make(map[string]chan *DataSpec)
	for _, stream := range inputstream.Name {
		inputch := make(chan *DataSpec, 1000)
		inputchannels[stream] = inputch
	}
	//data slice to save input
	inputs := NewDataSlice(inputstream)
	return &InputManager{
		InputStreams:  inputstream.Name,
		InputChannels: inputchannels,
		Inputs:        inputs,
	}
}

func (im *InputManager) HasInputStream(stream string) bool {
	_, ok := im.InputChannels[stream]
	return ok
}

func (im *InputManager) GetInputStreamChannel(stream string) (chan *DataSpec, bool) {
	inputch, ok := im.InputChannels[stream]
	return inputch, ok
}

func (im *InputManager) Subscribe() (bool, bool) {
	var allok bool = true
	var allclosed bool = true
	for id, stream := range im.InputStreams {
		ch, _ := im.GetInputStreamChannel(stream)
		inputdata, ok := <-ch
		allok = allok && ok
		allclosed = allclosed && !ok
		//put data in order
		//TODO:set data
		im.Inputs.Values[id] = inputdata
	}
	return allok, allclosed
}

func (im *InputManager) AddData(input *DataSpec, stream string) error {
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
