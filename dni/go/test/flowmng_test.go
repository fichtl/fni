package test

import (
	"log"
	"testing"

	flowmng "github.com/amianetworks/dni/sdk/flowmanager"
)

func TestDataSpecClone(t *testing.T) {
	d := flowmng.DataSpec{
		StreamName: "abc",
		Data:       []int{1, 2, 3},
	}
	d1 := d.Clone()
	d.Data.([]int)[0] = 5
	d.StreamName = "bcd"
	log.Println(*d1)
	log.Println(d)
}

func TestDataSpecReturn(t *testing.T) {
	d := flowmng.DataSpec{
		StreamName: "abc",
		Data:       []int{1, 2, 3},
	}
	d1 := Copy(d)
	d.Data.([]int)[0] = 5
	d.StreamName = "bcd"
	log.Println(d1)
	log.Println(d)
}

func Copy(d flowmng.DataSpec) flowmng.DataSpec {
	return d
}
