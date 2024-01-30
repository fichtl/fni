package graph

import (
	"fmt"
	"path/filepath"

	"github.com/amianetworks/dni/utils"
	"github.com/spf13/viper"
)

type GraphInputStreamUnit struct {
	Name string `mapstructure:"name"`
	// DB Input Stream
	DBUrl   string   `mapstructure:"db_url"`
	DBName  string   `mapstructure:"db_name"`
	DBTable string   `mapstructure:"db_table"`
	Tags    []string `mapstructure:"tags"`
	// PCAPFile Input Stream
	PcapFile string `mapstructure:"pcapfile"`
}

type GraphOutputStreamUnit struct {
	Name string `mapstructure:"name"`
	// DB output stream
	DBUrl   string `mapstructure:"db_url"`
	DBName  string `mapstructure:"db_name"`
	DBTable string `mapstructure:"db_table"`
	// File output stream
	File string `mapstructure:"file"`
}

// 节点的结构
type NodeInputStreamInfo struct {
	Window  string `mapstructure:"window"`
	WinSize string `mapstructure:"window_size"`
}

type NodeUnit struct {
	Runner          string              `mapstructure:"runner"`
	InputStream     []string            `mapstructure:"input_stream"`
	InputStreamInfo NodeInputStreamInfo `mapstructure:"input_stream_info"`
	OutputStream    []string            `mapstructure:"output_stream"`
}

// 图的结构
type GraphConf struct {
	GraphInputStream  []GraphInputStreamUnit  `mapstructure:"input_stream"`
	GraphOutputStream []GraphOutputStreamUnit `mapstructure:"output_stream"`
	Nodes             []NodeUnit              `mapstructure:"nodes"`
}

func GetGraphConfig(graphfile string) (*GraphConf, error) {
	fileName := filepath.Base(graphfile)
	filePath := filepath.Dir(graphfile)

	v := viper.New()
	v.SetConfigName(fileName)
	v.SetConfigType("yaml")
	v.AddConfigPath(filePath)

	if err := v.ReadInConfig(); err != nil {
		return nil, fmt.Errorf("wrong format: %v", err)
	}

	var gc GraphConf
	if err := v.Unmarshal(&gc); err != nil {
		return nil, err
	}
	return &gc, nil
}

// return sourceEdge and nodeEdge
func (gc *GraphConf) GetNodeEdge() (map[int][]int, error) {
	// 图的输入
	rootInput := make([]string, 0)
	for _, input := range gc.GraphInputStream {
		rootInput = append(rootInput, input.Name)
	}
	// 图的最终输出
	endOutput := make([]string, 0)
	for _, output := range gc.GraphOutputStream {
		endOutput = append(endOutput, output.Name)
	}
	// 图节点的输入要么从图输入来，要么从其他节点来
	nodeOutputs := make(map[string]int) // key=output stream name val=node source
	nodeInputs := make(map[string]struct{})
	for id, node := range gc.Nodes {
		for _, out := range node.OutputStream {
			nodeOutputs[out] = id
		}
		for _, in := range node.InputStream {
			nodeInputs[in] = struct{}{}
		}
	}
	nodeEdge := make(map[int][]int)
	// 检查每个节点的输入是否正确
	for id, node := range gc.Nodes {
		for _, in := range node.InputStream {
			// 判断是否为图输入
			// 是，继续判断下一节点；否，判断是否来源于node
			if utils.MatchStringSlice(in, rootInput) {
				continue
			}
			sourceID, ok := nodeOutputs[in]
			if !ok {
				// 输入即不是来源于图，也不是来源于其他节点
				// log.R.Error("runner (%s) input stream not found source (%s)", node.Runner, in)
				return nil, fmt.Errorf("runner (%s) input stream not found source (%s)", node.Runner, in)
			}
			if sourceID > id {
				// log.R.Error("Graph tasks have loops, please check nodeID %d and %d", id, sourceID)
				return nil, fmt.Errorf("graph tasks have loops, please check nodeID %d and %d", id, sourceID)
			}

			val, exist := nodeEdge[sourceID]
			if !exist {
				val = make([]int, 0)
			}
			if utils.MatchIntSlice(id, val) {
				continue
			}
			val = append(val, id)
			nodeEdge[sourceID] = val
		}
	}
	// 检查每个节点的输出是否都作为其他节点输入
	for out, id := range nodeOutputs {
		if utils.MatchStringSlice(out, endOutput) {
			continue
		}
		_, ok := nodeInputs[out]
		if !ok {
			// log.R.Error("nodeID (%d) output stream (%s) was not used", id, out)
			return nil, fmt.Errorf("nodeID (%d) output stream (%s) was not used", id, out)
		}
	}
	return nodeEdge, nil
}

func (gc *GraphConf) GetSourceEdge() (map[int][]int, error) {

	sourceEdge := make(map[int][]int)
	for nodeID, node := range gc.Nodes {
		for _, in := range node.InputStream {
			for index, graphIn := range gc.GraphInputStream {
				if in == graphIn.Name {
					src, ok := sourceEdge[index]
					if !ok {
						src = make([]int, 0)
					}
					src = append(src, nodeID)
					sourceEdge[index] = src
					break
				}
			}
		}
	}
	return sourceEdge, nil
}
