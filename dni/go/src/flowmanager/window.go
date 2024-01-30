package flowmanager

import (
	"strconv"
	"sync"
	"time"
)

const (
	DEFAULT_WINDOW         string = "defaultwindow"
	COUNT_WINDOW           string = "count"
	TUMBLING_WINDOWN       string = "tumblingwindow"
	DefaultSecondTimeDelay int64  = 1
)

type WindowStrategy interface {
	Trigger(data DataSpec) (bool, []DataSpec)
	BeforeWindow(data DataSpec) bool
}

type WindowOptions struct {
	Type    string
	Count   int
	WinSize time.Duration
}

func NewWindowOptions(wintype, winsize string) (WindowOptions, error) {
	winOpts := WindowOptions{
		Type: wintype,
	}
	switch wintype {
	case COUNT_WINDOW:
		count, err := strconv.ParseInt(winsize, 10, 64)
		if err != nil {
			return winOpts, err
		}
		winOpts.Count = int(count)
	case TUMBLING_WINDOWN:
		// TODO
	}
	return winOpts, nil
}

// 始终触发，数据来了直接进入数据通道
type DefaultWindow struct{}

func NewDefaultWindow() *DefaultWindow {
	return &DefaultWindow{}
}

func (dw *DefaultWindow) BeforeWindow(data DataSpec) bool {
	return false
}

func (dw *DefaultWindow) Trigger(data DataSpec) (bool, []DataSpec) {
	return true, []DataSpec{data}
}

// 执行数据条数，达到 Count 则触发
type CountWindow struct {
	dataCount  int
	boundCount int
	mutex      *sync.Mutex
	Inputs     []DataSpec
}

func NewCountWindow(count int) *CountWindow {
	return &CountWindow{
		boundCount: count,
		mutex:      &sync.Mutex{},
		Inputs:     make([]DataSpec, 0),
	}
}

func (cw *CountWindow) BeforeWindow(data DataSpec) bool {
	// countWindow不遵循时间过期
	return false
}

func (cw *CountWindow) Trigger(data DataSpec) (bool, []DataSpec) {
	cw.mutex.Lock()
	defer cw.mutex.Unlock()

	cw.dataCount++
	if cw.dataCount == cw.boundCount {
		// 数据条目满足要求，触发并归零
		result := append(cw.Inputs, data)
		cw.Inputs = make([]DataSpec, 0)
		cw.dataCount = 0
		return true, result
	}
	cw.Inputs = append(cw.Inputs, data)
	return false, nil
}

// 滚动窗口:固定时间窗口
type TumblingWindow struct {
	start, end int64 // 时间戳
	winSize    int64 // 时间窗口的大小
	mutex      *sync.Mutex
	Inputs     []DataSpec
	cache      []DataSpec
}

// TODO：窗口计算的起始时间可以指定
func NewTumblingWindow(winSize time.Duration) *TumblingWindow {
	return &TumblingWindow{
		winSize: int64(winSize.Seconds()),
		start:   time.Now().Unix(),
		mutex:   &sync.Mutex{},
		Inputs:  make([]DataSpec, 0),
		cache:   make([]DataSpec, 0),
	}
}

// 判断数据是否过期
func (tw *TumblingWindow) BeforeWindow(data DataSpec) bool {
	ts := data.TimeStamp.Unix()
	return ts < tw.start
}

// TODO: check
func (tw *TumblingWindow) Trigger(data DataSpec) (bool, []DataSpec) {
	tw.mutex.Lock()
	defer tw.mutex.Unlock()

	ts := data.TimeStamp.Unix()
	if ts < tw.end {
		tw.Inputs = append(tw.Inputs, data)
		return false, nil
	}
	if ts >= tw.end && ts-DefaultSecondTimeDelay < tw.end {
		// 说明是下一个时间区间
		tw.cache = append(tw.cache, data)
		return false, nil
	}
	// 窗口触发
	if ts-DefaultSecondTimeDelay >= tw.end {
		if ts < tw.end+tw.winSize {
			// 在下一个时间窗口内
			tw.cache = append(tw.cache, data)
			result := tw.Inputs
			tw.Inputs = tw.cache
			tw.cache = make([]DataSpec, 0)
			return true, result
		} else {
			// 时间窗口跳跃了
			// 跳过两个时间窗口，则忽略这条数据
			result := tw.Inputs
			tw.Inputs = tw.cache
			tw.cache = make([]DataSpec, 0)
			if ts == tw.end+tw.winSize {
				tw.cache = append(tw.cache, data)
			}
			return true, result
		}
	}

	tw.Inputs = append(tw.Inputs, data)
	return false, nil
}
