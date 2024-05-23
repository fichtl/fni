package graph

import (
	"fmt"
	"path/filepath"
	"strconv"
	"strings"

	gqueue "github.com/gogf/gf/v2/container/gqueue"
	"github.com/spf13/viper"
)

type NodeConf struct {
	Task         string      `mapstructure:"task"`
	InputStream  []string    `mapstructure:"input_stream"`
	OutputStream []string    `mapstructure:"output_stream"`
	Options      interface{} `mapstructure:"options"`
}

// Graph config
type GraphConf struct {
	GraphInputStream  []string   `mapstructure:"input_stream"`
	GraphOutputStream []string   `mapstructure:"output_stream"`
	Nodes             []NodeConf `mapstructure:"node"`
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

type TagIndex struct {
	Tag   string
	Index int
}

type StreamUnit struct {
	Name        []string
	TagIndexMap map[string]int
}

func NewStreamUnit(streams []string) (*StreamUnit, error) {
	su := &StreamUnit{
		Name:        make([]string, len(streams)),
		TagIndexMap: make(map[string]int),
	}
	for id, stream := range streams {
		tagindex, name, err := GetTagIndexName(stream, id)
		if err != nil {
			return nil, fmt.Errorf("wrong stream format:%v", err)
		}
		su.Name[id] = name
		su.TagIndexMap[tagindex] = id
	}
	return su, nil
}

type NodeUnit struct {
	InputStream  StreamUnit
	OutputStream StreamUnit
	Task         string
	Options      interface{}
}

func NewNodeUnit(nodeConf NodeConf) (*NodeUnit, error) {
	nu := &NodeUnit{
		Task:    nodeConf.Task,
		Options: nodeConf.Options,
	}
	inputstream, err := NewStreamUnit(nodeConf.InputStream)
	if err != nil {
		return nil, err
	}
	outputstream, err := NewStreamUnit(nodeConf.OutputStream)
	if err != nil {
		return nil, err
	}
	nu.InputStream = *inputstream
	nu.OutputStream = *outputstream
	return nu, nil
}

type GraphUnit struct {
	GraphInputStream  StreamUnit
	GraphOutputStream StreamUnit
	Nodes             []NodeUnit
}

func GetGraphUnit(gc *GraphConf) (*GraphUnit, error) {
	//graph input stream info
	gInputStream, err := NewStreamUnit(gc.GraphInputStream)
	if err != nil {
		return nil, err
	}
	//graph output stream info
	gOutputStream, err := NewStreamUnit(gc.GraphOutputStream)
	if err != nil {
		return nil, err
	}
	//node info
	nodes := make([]NodeUnit, len(gc.Nodes))
	for nid, nodeConf := range gc.Nodes {
		node, err := NewNodeUnit(nodeConf)
		if err != nil {
			return nil, err
		}
		nodes[nid] = *node
	}
	//graph unit
	gu := &GraphUnit{
		GraphInputStream:  *gInputStream,
		GraphOutputStream: *gOutputStream,
		Nodes:             nodes,
	}
	return gu, nil
}

func (gu *GraphUnit) GetNodeEdge() (map[int]map[string][]int, error) {
	//graph input
	gInput := make(map[string]struct{})
	for _, streamname := range gu.GraphInputStream.Name {
		gInput[streamname] = struct{}{}
	}
	//graph output
	gOutput := make(map[string]struct{})
	for _, streamname := range gu.GraphOutputStream.Name {
		gOutput[streamname] = struct{}{}
	}
	// inputs stream of node from pre node or graph input
	nodeOutputs := make(map[string]int)
	nodeInputs := make(map[string]struct{})
	outputEdge := make(map[int]map[string][]int)
	for id, node := range gu.Nodes {
		outputEdge[id] = make(map[string][]int)
		for _, out := range node.OutputStream.Name {
			if _, ok := nodeOutputs[out]; ok {
				return nil, fmt.Errorf("graph hase multi same output stream:%s", out)
			}
			nodeOutputs[out] = id
			outputEdge[id][out] = make([]int, 0)
		}
		nodeInputSet := make(map[string]struct{})
		for _, in := range node.InputStream.Name {
			if _, ok := nodeInputSet[in]; ok {
				return nil, fmt.Errorf("node %d hase multi same input stream:%s", id, in)
			}
			nodeInputSet[in] = struct{}{}
			nodeInputs[in] = struct{}{}
		}
	}
	//create output edge
	for id, node := range gu.Nodes {
		for _, stream := range node.InputStream.Name {
			// if from graph input
			if _, ok := gInput[stream]; ok {
				continue
			}
			preNodeID, ok := nodeOutputs[stream]
			if !ok {
				// if not from graph input or pre node
				return nil, fmt.Errorf("runner (%s) input stream not found source (%s)", node.Task, stream)
			}
			//TODO:loop check
			if preNodeID > id {
				return nil, fmt.Errorf("graph tasks have loops, please check nodeID %d and %d", id, preNodeID)
			}
			outputEdge[preNodeID][stream] = append(outputEdge[preNodeID][stream], id)
		}
	}

	//check graph outputs from node outputs
	for gOutStream := range gOutput {
		_, ok := nodeOutputs[gOutStream]
		if !ok {
			return nil, fmt.Errorf("graph output stream (%s) not found source", gOutStream)
		}
	}
	return outputEdge, nil
}

func (gu *GraphUnit) GetSourceEdge() (map[string][]int, error) {
	//gInput
	gInput := make(map[string]struct{})
	sourceEdge := make(map[string][]int)
	for _, graphIn := range gu.GraphInputStream.Name {
		gInput[graphIn] = struct{}{}
		sourceEdge[graphIn] = make([]int, 0)
	}
	//cretae source edge
	for nodeID, node := range gu.Nodes {
		for _, in := range node.InputStream.Name {
			if _, ok := gInput[in]; ok {
				sourceEdge[in] = append(sourceEdge[in], nodeID)
			}
		}
	}
	return sourceEdge, nil
}

func GetTagIndexName(stream string, pos int) (tagindex string, name string, err error) {
	var index int
	var tag string
	tag_name_index := strings.Split(stream, ":")
	if len(tag_name_index) == 3 {
		tag = tag_name_index[0]
		index, err = strconv.Atoi(tag_name_index[1])
		name = tag_name_index[2]
	} else if len(tag_name_index) == 2 {
		tag = tag_name_index[0]
		index = 0
		name = tag_name_index[1]
	} else if len(tag_name_index) == 1 {
		tag = ""
		index = pos
		name = tag_name_index[0]
	} else {
		err = fmt.Errorf("stream error")
	}
	tag_index := fmt.Sprintf("%s:%d", tag, index)
	return tag_index, name, err
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
