package config

import (
	"fmt"
	"path/filepath"
	"strconv"
	"strings"

	gqueue "github.com/gogf/gf/v2/container/gqueue"
	"github.com/spf13/viper"
)

type NodeConf struct {
	Task           string      `mapstructure:"task"`
	InputStream    []string    `mapstructure:"input_stream"`
	OutputStream   []string    `mapstructure:"output_stream"`
	InputSideData  []string    `mapstructure:"input_sidedata"`
	OutputSideData []string    `mapstructure:"output_sidedata"`
	Options        interface{} `mapstructure:"options"`
}

// Graph config
type GraphConf struct {
	GraphInputStream    []string   `mapstructure:"input_stream"`
	GraphOutputStream   []string   `mapstructure:"output_stream"`
	GraphInputSideData  []string   `mapstructure:"input_sidedata"`
	GraphOutputSideData []string   `mapstructure:"output_sidedata"`
	Nodes               []NodeConf `mapstructure:"node"`
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

type StreamUnit struct {
	Name        []string
	TagIndexMap map[string]int
	NameMap     map[string]int
}

func NewStreamUnit(streams []string) (*StreamUnit, error) {
	su := &StreamUnit{
		Name:        make([]string, len(streams)),
		TagIndexMap: make(map[string]int),
		NameMap:     make(map[string]int),
	}
	for id, stream := range streams {
		tagindex, name, err := GetTagIndexName(stream, id)
		if err != nil {
			return nil, fmt.Errorf("wrong stream format:%v", err)
		}
		su.Name[id] = name
		su.TagIndexMap[tagindex] = id
		su.NameMap[name] = id
	}
	return su, nil
}

type NodeUnit struct {
	InputStream    StreamUnit
	OutputStream   StreamUnit
	InputSideData  StreamUnit
	OutputSideData StreamUnit
	Task           string
	Options        interface{}
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
	inputSidedata, err := NewStreamUnit(nodeConf.InputSideData)
	if err != nil {
		return nil, err
	}
	outputSidedata, err := NewStreamUnit(nodeConf.OutputSideData)
	if err != nil {
		return nil, err
	}
	nu.InputStream = *inputstream
	nu.OutputStream = *outputstream
	nu.InputSideData = *inputSidedata
	nu.OutputSideData = *outputSidedata
	return nu, nil
}

type GraphUnit struct {
	GraphInputStream    StreamUnit
	GraphOutputStream   StreamUnit
	GraphInputSideData  StreamUnit
	GraphOutputSideData StreamUnit
	Nodes               []NodeUnit
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
	//graph input sidedata
	gInputSideData, err := NewStreamUnit(gc.GraphInputSideData)
	if err != nil {
		return nil, err
	}
	//graph output sidedata
	gOutputSideData, err := NewStreamUnit(gc.GraphOutputSideData)
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
		GraphInputStream:    *gInputStream,
		GraphOutputStream:   *gOutputStream,
		GraphInputSideData:  *gInputSideData,
		GraphOutputSideData: *gOutputSideData,
		Nodes:               nodes,
	}
	return gu, nil
}

func (gu *GraphUnit) GetNodeEdge() (map[int]map[string][]int, []int, error) {
	//graph inputs
	gInputs := make(map[string]struct{})
	for _, name := range gu.GraphInputStream.Name {
		gInputs[name] = struct{}{}
	}
	//graph outputs
	gOutputs := make(map[string]struct{})
	for _, name := range gu.GraphOutputStream.Name {
		gOutputs[name] = struct{}{}
	}

	//node Inputs & node Outputs & nodeEdge
	nodeInputs := make([][]string, len(gu.Nodes))
	nodeOutputs := make(map[string]int)
	nodeEdge := make(map[int]map[string][]int)
	for nid, node := range gu.Nodes {
		inputs := node.InputStream.Name
		outputs := node.OutputStream.Name
		nodeInputs[nid] = inputs
		//check node has multi different inputs
		inputsSet := make(map[string]struct{})
		for _, input := range inputs {
			if _, ok := inputsSet[input]; ok {
				return nil, nil, fmt.Errorf("node %d hase multi same input stream:%s", nid, input)
			}
			inputsSet[input] = struct{}{}
		}
		//check any graph output has different name
		nodeEdge[nid] = make(map[string][]int)
		for _, output := range outputs {
			if _, ok := nodeOutputs[output]; ok {
				return nil, nil, fmt.Errorf("graph hase multi same output stream:%s", output)
			}
			nodeOutputs[output] = nid
			nodeEdge[nid][output] = make([]int, 0)
		}
	}
	//check graph outputs from node outputs
	for output := range gOutputs {
		_, ok := nodeOutputs[output]
		if !ok {
			return nil, nil, fmt.Errorf("graph output (%s) not found source", output)
		}
	}
	//get node edge & preMap & nextMap
	preMap, nextMap, err := GetNodeEdge(gInputs, gOutputs, nodeInputs, nodeOutputs, nodeEdge)
	if err != nil {
		return nil, nil, err
	}
	//Loop Judge
	// log.Printf("preMap:%v", preMap)
	// log.Printf("nextMap:%v", nextMap)
	sortedNodes, isExistLoop := IsExistLoop(preMap, nextMap)
	if isExistLoop {
		return nil, nil, fmt.Errorf("graph has loop")
	}
	return nodeEdge, sortedNodes, nil
}

func (gu *GraphUnit) GetSideNodeEdge() (map[int]map[string][]int, []int, error) {
	//graph inputs
	gInputs := make(map[string]struct{})
	for _, name := range gu.GraphInputSideData.Name {
		gInputs[name] = struct{}{}
	}
	//graph outputs
	gOutputs := make(map[string]struct{})
	for _, name := range gu.GraphOutputSideData.Name {
		gOutputs[name] = struct{}{}
	}

	//node Inputs & node Outputs & nodeEdge
	nodeInputs := make([][]string, len(gu.Nodes))
	nodeOutputs := make(map[string]int)
	nodeEdge := make(map[int]map[string][]int)
	for nid, node := range gu.Nodes {
		inputs := node.InputSideData.Name
		outputs := node.OutputSideData.Name
		nodeInputs[nid] = inputs
		//check node has multi different inputs
		inputsSet := make(map[string]struct{})
		for _, input := range inputs {
			if _, ok := inputsSet[input]; ok {
				return nil, nil, fmt.Errorf("node %d hase multi same input stream:%s", nid, input)
			}
			inputsSet[input] = struct{}{}
		}
		//check any graph output has different name
		nodeEdge[nid] = make(map[string][]int)
		for _, output := range outputs {
			if _, ok := nodeOutputs[output]; ok {
				return nil, nil, fmt.Errorf("graph hase multi same output stream:%s", output)
			}
			nodeOutputs[output] = nid
			nodeEdge[nid][output] = make([]int, 0)
		}
	}
	//check graph outputs from node outputs
	for output := range gOutputs {
		_, ok := nodeOutputs[output]
		if !ok {
			return nil, nil, fmt.Errorf("graph output (%s) not found source", output)
		}
	}
	//get node edge & preMap & nextMap
	preMap, nextMap, err := GetNodeEdge(gInputs, gOutputs, nodeInputs, nodeOutputs, nodeEdge)
	if err != nil {
		return nil, nil, err
	}
	//Loop Judge
	// log.Printf("preMap:%v", preMap)
	// log.Printf("nextMap:%v", nextMap)
	sortedNodes, isExistLoop := IsExistLoop(preMap, nextMap)
	if isExistLoop {
		return nil, nil, fmt.Errorf("graph has loop")
	}
	return nodeEdge, sortedNodes, nil
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

func (gu *GraphUnit) GetSideSourceEdge() (map[string][]int, error) {
	//gInput
	gInput := make(map[string]struct{})
	sourceEdge := make(map[string][]int)
	for _, graphIn := range gu.GraphInputSideData.Name {
		gInput[graphIn] = struct{}{}
		sourceEdge[graphIn] = make([]int, 0)
	}
	//cretae source edge
	for nodeID, node := range gu.Nodes {
		for _, in := range node.InputSideData.Name {
			if _, ok := gInput[in]; ok {
				sourceEdge[in] = append(sourceEdge[in], nodeID)
			}
		}
	}
	return sourceEdge, nil
}

func GetNodeEdge(gInputsMap, gOutputsMap map[string]struct{}, nodeInputs [][]string, nodeOutputs map[string]int, nodeEdge map[int]map[string][]int) (preMap map[int]map[int]struct{}, nextMap map[int]map[int]struct{}, err error) {
	//init results
	preMap = make(map[int]map[int]struct{})
	nextMap = make(map[int]map[int]struct{})
	for nid := 0; nid < len(nodeInputs); nid++ {
		preMap[nid] = make(map[int]struct{})
		nextMap[nid] = make(map[int]struct{})
	}
	//get node edge
	for nid, inputs := range nodeInputs {
		for _, input := range inputs {
			if _, ok := gInputsMap[input]; ok {
				continue
			}
			preNID, ok := nodeOutputs[input]
			if !ok {
				return preMap, nextMap, fmt.Errorf("node[%d] input (%s) can not find source", nid, input)
			}
			nodeEdge[preNID][input] = append(nodeEdge[preNID][input], nid)
			preMap[nid][preNID] = struct{}{}
			nextMap[preNID][nid] = struct{}{}
		}
	}
	return preMap, nextMap, nil
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

func IsExistLoop(preList, nextList map[int]map[int]struct{}) ([]int, bool) {
	nodeNumber := len(preList)
	nodeCount := make([]int, nodeNumber)
	sortedNodes := make([]int, 0)
	q := gqueue.New()
	for nid := range nodeCount {
		preNodes := preList[nid]
		if len(preNodes) == 0 {
			q.Push(nid)
		}
		nodeCount[nid] = len(preNodes)
	}
	zeroCount := 0
	for {
		if q.Len() == 0 {
			break
		}
		//pop node
		node := q.Pop()
		sortedNodes = append(sortedNodes, node.(int))
		zeroCount += 1
		nextNodes := nextList[node.(int)]
		if len(nextNodes) == 0 {
			continue
		}
		for nnode := range nextNodes {
			nodeCount[nnode] -= 1
			if nodeCount[nnode] == 0 {
				//if node has no pre node
				q.Push(nnode)
			}
		}
	}
	return sortedNodes, zeroCount != nodeNumber
}
