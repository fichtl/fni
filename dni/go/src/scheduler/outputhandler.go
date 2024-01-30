package scheduler

import (
	"fmt"
	"os"

	"github.com/amianetworks/am.modules/db/src/tsdb"
	flowmng "github.com/amianetworks/dni/src/flowmanager"
	"github.com/amianetworks/dni/src/graph"
)

type OutputStreamHandler interface {
	PrepareForRun() error
	SendToGraphOutput(d flowmng.DataSpec) error
	Close() error
}

type GraphDBOutput struct {
	Name string

	DBName  string
	DBUrl   string
	DBTable string

	DBHandle   tsdb.Database
	CollHandle tsdb.Collection
}

func InitGraphDBOutput(out graph.GraphOutputStreamUnit) *GraphDBOutput {
	return &GraphDBOutput{
		Name:    out.Name,
		DBName:  out.DBName,
		DBUrl:   out.DBUrl,
		DBTable: out.DBTable,
	}
}

func (h *GraphDBOutput) PrepareForRun() error {
	// 初始化handle
	return nil
}

func (h *GraphDBOutput) SendToGraphOutput(d flowmng.DataSpec) error {
	realdata := d.Data.(tsdb.Point)
	if err := h.CollHandle.AddDataPoint(&realdata); err != nil {
		return err
	}
	return nil
}

func (h *GraphDBOutput) Close() error {
	return h.DBHandle.Destroy()
}

type GraphFileOutput struct {
	Name     string
	FileName string
	Handle   *os.File
}

func InitGraphFileOutput(out graph.GraphOutputStreamUnit) *GraphFileOutput {
	return &GraphFileOutput{
		Name:     out.Name,
		FileName: out.File,
	}
}

func (h *GraphFileOutput) PrepareForRun() error {
	filehandle, err := os.OpenFile(h.FileName, os.O_CREATE|os.O_RDWR|os.O_APPEND, 0664)
	if err != nil {
		return err
	}
	h.Handle = filehandle
	return nil
}

func (h *GraphFileOutput) SendToGraphOutput(d flowmng.DataSpec) error {
	h.Handle.Truncate(0)
	switch d.Type {
	case flowmng.DATA_TYPE_INT:
		realdata := d.Data.(int)
		if _, err := h.Handle.WriteString(fmt.Sprint(realdata)); err != nil {
			return err
		}
	}
	return nil
}

func (h *GraphFileOutput) Close() error {
	h.Handle.Close()
	return nil
}
