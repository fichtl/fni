package scheduler

import (
	"fmt"
	"strings"
	"time"

	"github.com/amianetworks/am.modules/db/src/tsdb"
	"github.com/amianetworks/am.modules/log"
	flowmng "github.com/amianetworks/dni/src/flowmanager"
	"github.com/amianetworks/dni/src/graph"
	"github.com/google/gopacket/pcap"
)

type InputStreamHandler interface {
	PrepareForRun() error
	Close() error
	GenerateInputStream() error
	GetDataChannel() chan flowmng.DataSpec
}

type GraphPCAPFileInput struct {
	Name     string
	FileName string
	Handle   *pcap.Handle

	DataChan chan flowmng.DataSpec
}

func InitPCAPFileHandler(in graph.GraphInputStreamUnit) *GraphPCAPFileInput {
	return &GraphPCAPFileInput{
		Name:     in.Name,
		FileName: in.PcapFile,

		DataChan: make(chan flowmng.DataSpec, 1000),
	}
}

func (h *GraphPCAPFileInput) PrepareForRun() error {
	pcaphandle, err := pcap.OpenOffline(h.FileName)
	if err != nil {
		return err
	}
	h.Handle = pcaphandle
	return nil
}

func (h *GraphPCAPFileInput) Close() error {
	h.Handle.Close()
	close(h.DataChan)
	for range h.DataChan {
	}
	return nil
}

func (h *GraphPCAPFileInput) GenerateInputStream() error {
	d := flowmng.DataSpec{
		Type:      flowmng.DATA_TYPE_PCAP_HANDLE,
		TimeStamp: time.Now(),
		Data:      h.Handle,
	}
	h.DataChan <- d
	return nil
}

func (h *GraphPCAPFileInput) GetDataChannel() chan flowmng.DataSpec {
	return h.DataChan
}

func (h *GraphPCAPFileInput) GetInputStreamName() string {
	return h.Name
}

type GraphDBInput struct {
	Name    string
	DBName  string
	DBUrl   string
	DBTable string
	Tags    map[string]string

	Query      tsdb.Query
	DBHandle   tsdb.Database
	CollHandle tsdb.Collection

	DataChan chan flowmng.DataSpec

	tick *time.Ticker
	stop chan struct{}
}

func InitDBHandler(in graph.GraphInputStreamUnit) *GraphDBInput {
	inHandle := &GraphDBInput{
		Name:     in.Name,
		DBName:   in.DBName,
		DBUrl:    in.DBUrl,
		DBTable:  in.DBTable,
		DataChan: make(chan flowmng.DataSpec, 1000),
	}
	for _, tag := range in.Tags {
		kv := strings.Split(tag, ":")
		inHandle.Tags[kv[0]] = kv[1]
	}
	return inHandle
}

func (h *GraphDBInput) PrepareForRun() error {
	// 初始化query和collhandle
	return nil
}

func (h *GraphDBInput) Close() error {
	if h.tick != nil {
		close(h.stop)
	}
	close(h.DataChan)
	for range h.DataChan {
	}
	return h.DBHandle.Destroy()
}

func (h *GraphDBInput) GenerateInputStream() error {
	if h.tick != nil {
		return fmt.Errorf("source flow has been started")
	}
	h.tick = time.NewTicker(1 * time.Second)
	h.stop = make(chan struct{})
	go func() {
		for {
			select {
			case <-h.tick.C:
				points, err := h.CollHandle.FindDataPoints(&h.Query)
				if err != nil {
					h.tick.Stop()
					h.tick = nil
					close(h.stop)
					log.R.Errorf("generate input stream failed, err=%v", err)
					return
				}
				d := flowmng.DataSpec{
					Type:      flowmng.DATA_TYPE_QUERY_RESULT,
					TimeStamp: time.Now(),
					Data:      points,
				}
				h.DataChan <- d
			case <-h.stop:
				h.tick.Stop()
				h.tick = nil
				return
			}
		}
	}()
	return nil
}

func (h *GraphDBInput) GetDataChannel() chan flowmng.DataSpec {
	return h.DataChan
}

func (h *GraphDBInput) GetInputStreamName() string {
	return h.Name
}
