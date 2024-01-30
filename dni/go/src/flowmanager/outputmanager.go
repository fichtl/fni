package flowmanager

// 每个节点的输出流直接进入output manager数据通道
type OutputManager struct {
	Output chan DataSpec
}

func NewOutputManager() *OutputManager {
	return &OutputManager{
		Output: make(chan DataSpec, 1000),
	}
}
