package flowmanager

import (
	"time"

	"github.com/amianetworks/am.modules/log"
)

const (
	DATA_TYPE_NONE         string = "none"
	DATA_TYPE_INT          string = "int"
	DATA_TYPE_INT_SLICE    string = "intslice"
	DATA_TYPE_SND_CONFIG   string = "sndServerConfig"
	DATA_TYPE_PCAP_HANDLE  string = "pcaphandler"
	DATA_TYPE_QUERY_RESULT string = "queryResult"
)

type DataSpec struct {
	NodeID    int
	Type      string
	TimeStamp time.Time
	Data      interface{}
}

type InputManager struct {
	Input  chan []DataSpec
	Window WindowStrategy
}

func NewInputManager(wops WindowOptions) *InputManager {

	var wins WindowStrategy
	switch wops.Type {
	case COUNT_WINDOW:
		wins = NewCountWindow(wops.Count)
	case TUMBLING_WINDOWN:
		wins = NewTumblingWindow(wops.WinSize)
	default:
		wins = NewDefaultWindow()
	}

	return &InputManager{
		Input:  make(chan []DataSpec, 1000),
		Window: wins,
	}
}

func (im *InputManager) Subscribe() chan []DataSpec {
	return im.Input
}

func (im *InputManager) AddPacket(input DataSpec) {
	if im.Window.BeforeWindow(input) {
		// 过期数据直接丢弃
		return
	}
	trigger, inputs := im.Window.Trigger(input)
	if trigger {
		if inputs == nil {
			log.R.Warn("data flow is empty")
			return
		}
		if len(inputs) == 0 {
			// 这段时间窗口内没有数据流入
			return
		}
		im.Input <- inputs
	}
}

func (im *InputManager) Close() {
	close(im.Input)
	for range im.Input {
	}
}

func (om *OutputManager) Subscribe() chan DataSpec {
	return om.Output
}

func (om *OutputManager) Close() {
	close(om.Output)
	for range om.Output {
	}
}
