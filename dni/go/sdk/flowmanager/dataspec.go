package flowmanager

import "time"

const (
	DATA_TYPE_NONE         string = "none"
	DATA_TYPE_INT          string = "int"
	DATA_TYPE_INT_SLICE    string = "intslice"
	DATA_TYPE_SND_CONFIG   string = "sndServerConfig"
	DATA_TYPE_PCAP_HANDLE  string = "pcaphandler"
	DATA_TYPE_QUERY_RESULT string = "queryResult"
	//TODO:onnx tensor
	DATA_TYPE_FLOAT_2D_SLICE string = "float2Dslice"
	DATA_TYPE_TENSOR_FLOAT   string = "tensorfloat"
)

// StreamName用于算子输入输出匹配
type DataSpec struct {
	StreamName string
	Type       string
	TimeStamp  time.Time
	Data       interface{}
}

func (d *DataSpec) Clone() DataSpec {
	ds := make([]DataSpec, 0)
	ds = append(ds, *d)
	return ds[0]
}
