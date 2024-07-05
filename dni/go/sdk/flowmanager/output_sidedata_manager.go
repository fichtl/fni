package flowmanager

import (
	"fmt"

	config "github.com/amianetworks/dni/sdk/config"
)

type OutputSideDataManager struct {
	OutputSidedata    []string
	NextInputSideData map[string][]*DataSlice
	Outputs           *DataSlice
}

func NewOutputSideDataManager(outputSideData config.StreamUnit) *OutputSideDataManager {
	outputs := NewDataSlice(outputSideData)
	return &OutputSideDataManager{
		OutputSidedata: outputSideData.Name,
		Outputs:        outputs,
	}
}

func (osm *OutputSideDataManager) AddData(outputSideData *DataSpec, sidedata string) error {
	inputSideDataSlices, ok := osm.NextInputSideData[sidedata]
	if !ok {
		return fmt.Errorf("sidedata (%s) not exist", sidedata)
	}
	for id := 0; id < len(inputSideDataSlices); id++ {
		if id != 0 {
			//copy
			outputSideData = outputSideData.Clone()
		}
		//change next node input sidedata.Data
		inputSideDataSlices[id].Set(sidedata, outputSideData)
	}
	return nil
}

func (osm *OutputSideDataManager) AddAllData() {
	for id, sidedata := range osm.OutputSidedata {
		outputs := osm.Outputs.Values
		osm.AddData(outputs[id], sidedata)
	}
}

func (osm *OutputSideDataManager) Close() error {
	return nil
}
