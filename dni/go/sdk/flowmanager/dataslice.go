package flowmanager

import (
	"fmt"
	"log"

	config "github.com/amianetworks/dni/sdk/config"
)

type DataSlice struct {
	TagIndexMap map[string]int //key:tag+index val:pos
	NameMap     map[string]int
	Names       []string
	Values      []*DataSpec
	Size        int
}

func NewDataSlice(streamUnit config.StreamUnit) *DataSlice {
	size := len(streamUnit.TagIndexMap)
	ds := &DataSlice{
		TagIndexMap: streamUnit.TagIndexMap,
		NameMap:     streamUnit.NameMap,
		Names:       streamUnit.Name,
		Values:      make([]*DataSpec, size),
		Size:        size,
	}
	for _, pos := range streamUnit.NameMap {
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
		log.Printf("tag(%s)&index(%d) error", tag, index)
		return &DataSpec{}
	}
	if pos >= ds.Size {
		return &DataSpec{}
	}
	return ds.Values[pos]
}

func (ds *DataSlice) GetByName(name string) (*DataSpec, bool) {
	pos, ok := ds.NameMap[name]
	if !ok {
		return nil, ok
	}
	return ds.Values[pos], ok
}

func (ds *DataSlice) GetByTagIndex(tagindex string) *DataSpec {
	pos, ok := ds.TagIndexMap[tagindex]
	if !ok {
		return &DataSpec{}
	}
	if pos >= ds.Size {
		return &DataSpec{}
	}
	return ds.Values[pos]
}

func (ds *DataSlice) Set(name string, data *DataSpec) error {
	pos, ok := ds.NameMap[name]
	if !ok {
		return fmt.Errorf("name error")
	}
	ds.Values[pos] = data
	return nil
}

func (ds *DataSlice) HasName(name string) bool {
	_, ok := ds.NameMap[name]
	return ok
}

func (ds *DataSlice) Reset() {
	for pos := 0; pos < ds.Size; pos++ {
		ds.Values[pos] = &DataSpec{}
	}
}
