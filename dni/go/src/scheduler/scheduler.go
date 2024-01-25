package scheduler

import (
	"sync"

	"github.com/amianetworks/dni/src/data"
)

type InputStreamHandler interface {
	// InitializeInputStreamManagers()
	PrepareForRun() error
	ReceiveInputStreams(*data.InputManager) error
	// SetHeader()
	// SetMaxQueueSize()
	// AddPackets()
	// MovePackets()
	// SetNextTimestampBound()
	Close() error
	// SetBatchSize()
	// SetProcessTimestampBounds()
	// SetStreamContents()
	// GetNodeReadiness()
	// FillInputStreamSetByTimestamp()
}

type OutputStreamHandler interface {
	// InitializeOutputStreamManagers()
	PrepareForRun() error
	SendOutputStreams(data data.DataSpec) error
	// Open()
	// PrepareOutputs()
	// UpdateTaskTimestamp()
	// PostProcess()
	Close() error
	// AddMirror()
	// PropagateOutputPackets()
	// PropagateHeaderToMirrors()
	// PropagateOutputPacketsToMirrors()
}

type InputStream interface {
	Value()
	IsEmpty()
	IsDone()
}

type OutputStream interface {
	AddPacket()
	SetNextTimestampBound()
	SetOffset()
	SetHeader()
	Close()
}

type OutputStreamManager struct {
	Mutex sync.Mutex
}

type Executor interface {
	Start(data data.DataSpec) error
	Stop() error
}
