package flowmanager

import (
	"fmt"
	"log"
)

// Manage node output to next nodes/graph output input
type OutputManager struct {
	NextInputManagers map[string][]*InputManager //(key:output_stream name value:next nodes/graph output InputManager)
	OutputStreams     []string
}

func NewOutputManager(outputstream []string) *OutputManager {
	outputchannels := make(map[string]chan DataSpec)
	for _, stream := range outputstream {
		outputchannels[stream] = make(chan DataSpec, 1000)
	}
	return &OutputManager{
		OutputStreams: outputstream,
	}
}

func (om *OutputManager) AddData(output DataSpec, stream string) error {
	inMngs, ok := om.NextInputManagers[stream]
	if !ok {
		return fmt.Errorf("outputmanager stream (%s) not exist", stream)
	}
	for id := 0; id < len(inMngs); id++ {
		if id != 0 {
			//copy
			output = output.Clone()
		}
		mng := inMngs[id]
		err := mng.AddData(output, stream)
		if err != nil {
			return fmt.Errorf("outputmanager send stream (%s) to next node failed", stream)
		}
	}
	return nil
}

func (om *OutputManager) AddAllData(outputs []DataSpec) {
	for id := 0; id < len(outputs); id++ {
		stream := om.OutputStreams[id]
		outputs[id].StreamName = stream
		om.AddData(outputs[id], stream)
		log.Printf("send stream (%s) to next node", stream)
	}
}

func (om *OutputManager) Close() error {
	for stream, inMngs := range om.NextInputManagers {
		for _, mng := range inMngs {
			if err := mng.CloseInputChannel(stream); err != nil {
				log.Printf("close next node input failed, err=%v", err)
			}
		}
		log.Printf("stream (%s) to next node channel has been closed", stream)
	}
	return nil
}
