package scheduler

import (
	"fmt"

	"github.com/amianetworks/am.modules/log"
	"github.com/amianetworks/dni/src/data"
	"github.com/amianetworks/dni/src/design"
	"github.com/amianetworks/dni/src/graph"
)

type Node struct {
	ParentGraph *Graph
	NodeConf    graph.NodeConfig

	NodeName   string
	NodeID     int
	From       string
	LastOutput data.DataSpec

	InputHandler  InputStreamHandler
	OutputHandler OutputStreamHandler

	Input  *data.InputManager
	Output *data.OutputManager

	ExecutorStart StartExecutor
	ExecutorStop  StopExecutor

	Executor Executor

	stop chan struct{}
}

type StartExecutor func(input []data.DataSpec, option data.DataSpec) (data.DataSpec, error)
type StopExecutor func(info data.DataSpec) error

func (n *Node) PrepareForRun() error {
	// 检查input manager
	if n.Input == nil {
		return fmt.Errorf("node input manager is not ready")
	}
	// 检查output manager
	if n.Output == nil {
		return fmt.Errorf("node outout manage is not ready")
	}
	// 检查 start executor
	if n.ExecutorStart == nil {
		return fmt.Errorf("node task start executor is not ready")
	}
	// 检查 stop executor
	if n.ExecutorStop == nil {
		return fmt.Errorf("node task stop executor is not ready")
	}

	if err := n.InputHandler.PrepareForRun(); err != nil {
		return err
	}
	if err := n.OutputHandler.PrepareForRun(); err != nil {
		return err
	}
	return nil
}

func (n *Node) GenerateInputStream() error {
	err := n.InputHandler.ReceiveInputStreams(n.Input)
	if err != nil {
		return err
	}
	// switch n.From {
	// case design.INPUT_FROM_NODES:
	// 	// input from other nodes
	// 	// 则只需要建立数据管道，等待数据流入
	// case design.INPUT_FROM_PCAP_FILE:
	// 	// input from pcapfile
	// 	// input data为pcap文件的handler
	// 	// 遍历handler，然后针对handler处理数据
	// 	// 通常是解析pcapfile，pcapfile解析完则再无数据流入
	// 	err := n.InputHandler.ReceiveInputStreams(n.Input)
	// 	if err != nil {
	// 		return err
	// 	}
	// 	// 只有一个handler，所以不需要使用goroutine
	// 	// n.Input.Input <- data
	// case design.INPUT_FROM_DATABASE:
	// 	// PrepareForRun的时候获取数据的client
	// 	// InitializeInputManagerData()根据client去获取数据
	// 	// 获取数据的规则由taskstart根据任务类型自行决定
	// 	// 数据是通过数据库源源不断流入的，因此此处为一个协程
	// 	// err := n.InputHandler.ReceiveInputStreams(n.Input)
	// 	// if err != nil {
	// 	// 	return err
	// 	// }
	// 	// // 以下仅为Type为 CHAN_INT 示例
	// 	// // 或者将传输的channel放置在inputHandler内，这样不用数据转换
	// 	// d := data.Data.(chan int)
	// 	// go func() {
	// 	// 	for i := range d {
	// 	// 		input := data.DataSpec{
	// 	// 			Type: data.DATA_TYPE_INT,
	// 	// 			Data: i,
	// 	// 		}
	// 	// 		n.Input.Input <- input
	// 	// 	}
	// 	// }()
	// case design.INPUT_FROM_SND_SERVER_CONFIG:
	// 	// 数据采集任务在其他节点执行，只需要将命令传递到其他节点即可
	// 	// 不需要额外输入
	// 	if err := n.InputHandler.ReceiveInputStreams(n.Input); err != nil {
	// 		return err
	// 	}
	// }
	return nil
}

func (n *Node) Execute() error {

	n.stop = make(chan struct{})
	// 抽出
	if err := n.GenerateInputStream(); err != nil {
		return err
	}
	for {
		select {
		case data := <-n.Input.Input:
			log.R.Info("data: ", data)
			output, err := n.ExecutorStart(data, n.LastOutput) // Executor会去判断data类型，然后做计算
			if err != nil {
				return err
			}
			// 抽出，进入图处理
			if n.NodeConf.OutputStream.To == design.OUTPUT_TO_NODES {
				n.Output.Output <- output
			} else {
				if err := n.OutputHandler.SendOutputStreams(output); err != nil {
					return err
				}
			}
			n.LastOutput = output
		case <-n.stop:
			// TODO: 执行stop
			// 暂时停止向n.Input.Input传递数据
			if err := n.stopExecute(); err != nil {
				log.R.Errorf("(%s) stop executor failed, err=%v", n.NodeName, err)
			}

			return nil
		}
	}
}

func (n *Node) Clean() error {

	var tmpErr error
	if err := n.InputHandler.Close(); err != nil {
		tmpErr = err
	}
	if err := n.OutputHandler.Close(); err != nil {
		tmpErr = err
	}

	close(n.Input.Input)
	for range n.Input.Input {
	}
	close(n.Output.Output)
	for range n.Output.Output {
	}

	return tmpErr
}

func (n *Node) stopExecute() error {
	info := data.DataSpec{}
	switch n.NodeConf.Runner {
	case design.TELEMETRY_RUNNER:
		info.Type = data.DATA_TYPE_SND_CONFIG
		info.Data = n.NodeConf.InputStream.SNDServer.ServerAddrs
	case design.DATA_CLEANING_RUNNER:
	case design.DATA_STATISTICS_RUNNER:
	case design.DATA_SUM_RUNNER:
	}
	if err := n.ExecutorStop(info); err != nil {
		return err
	}
	return nil
}
