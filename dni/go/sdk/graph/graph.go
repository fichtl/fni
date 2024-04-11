package graph

import (
	"fmt"
	"path/filepath"

	"github.com/amianetworks/dni/sdk/utils"
	gqueue "github.com/gogf/gf/v2/container/gqueue"
	"github.com/spf13/viper"
)

type GraphInputStreamUnit struct {
	Name string `mapstructure:"name"`
}

type GraphOutputStreamUnit struct {
	Name string `mapstructure:"name"`
}

type NodeUnit struct {
	Runner        string                 `mapstructure:"runner"`
	InputStream   []string               `mapstructure:"input_stream"`
	OutputStream  []string               `mapstructure:"output_stream"`
	RunnerOptions map[string]interface{} `mapstructure:"options"`
}

// Graph config
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

func (gc *GraphConf) GetOutputEdge() (map[int]map[string][]int, error) {
	// graph input
	rootInput := make([]string, 0)
	for _, input := range gc.GraphInputStream {
		rootInput = append(rootInput, input.Name)
	}
	// graph output
	endOutput := make([]string, 0)
	for _, output := range gc.GraphOutputStream {
		endOutput = append(endOutput, output.Name)
	}
	// inputs stream of node from prenode or graph input
	nodeOutputs := make(map[string]int)
	nodeInputs := make(map[string]struct{})
	outputEdge := make(map[int]map[string][]int)
	for id, node := range gc.Nodes {
		outputEdge[id] = make(map[string][]int)
		for _, out := range node.OutputStream {
			if _, ok := nodeOutputs[out]; ok {
				return nil, fmt.Errorf("graph hase multi same output stream:%s", out)
			}
			nodeOutputs[out] = id
			outputEdge[id][out] = make([]int, 0)
		}
		nodeInputSet := make(map[string]struct{})
		for _, in := range node.InputStream {
			if _, ok := nodeInputSet[in]; ok {
				return nil, fmt.Errorf("node %d hase multi same input stream:%s", id, in)
			}
			nodeInputSet[in] = struct{}{}
			nodeInputs[in] = struct{}{}
		}
	}
	//create output edge
	for id, node := range gc.Nodes {
		for _, stream := range node.InputStream {
			// if from graph input
			if utils.MatchStringSlice(stream, rootInput) {
				continue
			}
			sourceID, ok := nodeOutputs[stream]
			if !ok {
				// if not from graph input or pre node
				return nil, fmt.Errorf("runner (%s) input stream not found source (%s)", node.Runner, stream)
			}
			//TODO:loop check
			if sourceID > id {
				return nil, fmt.Errorf("graph tasks have loops, please check nodeID %d and %d", id, sourceID)
			}

			outputEdge[sourceID][stream] = append(outputEdge[sourceID][stream], id)
		}
	}

	//check graph outputs from node outputs
	for _, gOutStream := range endOutput {
		_, ok := nodeOutputs[gOutStream]
		if !ok {
			return nil, fmt.Errorf("graph output stream (%s) not found source", gOutStream)
		}
	}
	return outputEdge, nil
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

// TODO: TOPO-Sort
func ISExistLoop(preqList map[int][]int, nextList map[int][]int, nodeNum int) bool {
	nodeCount := make([]int, nodeNum)
	q := gqueue.New()
	for node := range nodeCount {
		preNodes, ok := preqList[node]
		if !ok {
			q.Push(node)
		}
		nodeCount[node] = len(preNodes)
	}
	zeroCount := 0
	for {
		if q.Len() == 0 {
			break
		}
		node := q.Pop()
		zeroCount += 1
		nextNodes, ok := nextList[node.(int)]
		if !ok {
			continue
		}
		for _, nnode := range nextNodes {
			nodeCount[nnode] -= 1
			if nodeCount[nnode] == 0 {
				q.Push(nnode)
			}
		}
	}

	return zeroCount != nodeNum
}
