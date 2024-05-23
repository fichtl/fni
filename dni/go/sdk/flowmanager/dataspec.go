package flowmanager

import "time"

type DataSpec struct {
	StreamName string
	Type       string
	TimeStamp  time.Time
	Data       interface{}
}

func (d *DataSpec) Clone() *DataSpec {
	ds := make([]DataSpec, 0)
	ds = append(ds, *d)
	return &ds[0]
}
