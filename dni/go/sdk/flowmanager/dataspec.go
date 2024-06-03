package flowmanager

import (
	"time"

	"github.com/mohae/deepcopy"
)

type DataSpec struct {
	StreamName string
	Type       string
	TimeStamp  time.Time
	Data       interface{}
}

func (d *DataSpec) Clone() *DataSpec {
	dsCopy := deepcopy.Copy(*d)
	dsNew := dsCopy.(DataSpec)
	return &dsNew
}
