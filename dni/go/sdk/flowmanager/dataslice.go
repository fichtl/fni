package flowmanager

import (
	"fmt"
	"log"
)

type DataSlice struct {
	TagIndexMap map[string]int //key:tag+index val:pos
	Values      []*DataSpec
	Size        int
}

func NewDataSlice(tagIndexMap map[string]int) *DataSlice {
	ds := &DataSlice{
		TagIndexMap: tagIndexMap,
		Values:      make([]*DataSpec, len(tagIndexMap)),
		Size:        len(tagIndexMap),
	}
	for pos := 0; pos < len(tagIndexMap); pos++ {
		ds.Values[pos] = &DataSpec{}
	}
	return ds
}

func (ds *DataSlice) Get(tag string, index int) *DataSpec {
	if tag == "" {
		return ds.Values[index]
	}
	tag_index := fmt.Sprintf("%s:%d", tag, index)
	pos, ok := ds.TagIndexMap[tag_index]
	if !ok {
		log.Printf("tag & index error")
		return &DataSpec{}
	}
	if pos >= ds.Size {
		log.Printf("pos error")
		return &DataSpec{}
	}
	return ds.Values[pos]
}
