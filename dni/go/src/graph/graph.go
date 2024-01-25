package graph

import (
	"fmt"
	"path/filepath"
	"strconv"
	"strings"

	"github.com/amianetworks/dni/src/design"
	"github.com/spf13/viper"
)

/*
--------------------------

	External API

--------------------------
*/
func GetGraphConfig(graphfile string) (*GraphConfig, error) {
	gh, err := prepareGraphHandler(graphfile)
	if err != nil {
		return nil, err
	}
	gc, err := gh.parseGraphConfig()
	if err != nil {
		return nil, err
	}
	if err := gc.valid(); err != nil {
		return nil, err
	}
	return gc, nil
}

/*
--------------------------

	解读输入的graph文件

--------------------------
*/

type graphHandler struct {
	v *viper.Viper
}

func prepareGraphHandler(graphfile string) (*graphHandler, error) {
	fileName := filepath.Base(graphfile)
	filePath := filepath.Dir(graphfile)

	v := viper.New()
	v.SetConfigName(fileName)
	v.SetConfigType("yaml")
	v.AddConfigPath(filePath)

	if err := v.ReadInConfig(); err != nil {
		return nil, fmt.Errorf("wrong format: %v", err)
	}
	return &graphHandler{v: v}, nil
}

/*
-----------------------

	图的基本关键字解析

-----------------------
*/
type HostInfo struct {
	HostUrl   string
	Manage    string
	Protected []string
}

type Database struct {
	DBUrl   string
	DBName  string
	DBTable string
}

func (gh *graphHandler) readGraphInputStream() string {
	return strings.ToLower(gh.v.GetString("input_stream"))
}
func (gh *graphHandler) readGraphOutputStream() string {
	return strings.ToLower(gh.v.GetString("output_stream"))
}
func (gh *graphHandler) readGraphNodesNum() int { return gh.v.GetInt("nodes_num") }
func (gh *graphHandler) readNodeRunner(i int) string {
	key := fmt.Sprintf("node%d.runner", i)
	return gh.v.GetString(key)
}
func (gh *graphHandler) readInputStreamFrom(i int) string {
	key := fmt.Sprintf("node%d.input_stream.from", i)
	return strings.ToLower(gh.v.GetString(key))
}
func (gh *graphHandler) readInputStreamPcapFile(i int) design.PcapFileConfig {
	key := fmt.Sprintf("node%d.input_stream.pcapfile", i)
	return design.PcapFileConfig{
		PcapFile: gh.v.GetString(key),
	}
}
func (gh *graphHandler) readInputStreamNodes(i int) []string {
	key := fmt.Sprintf("node%d.input_stream.nodes_input", i)
	return gh.v.GetStringSlice(key)
}
func (gh *graphHandler) readInputStreamSNDConfig(i int) design.SNDServerConfig {
	urlk := fmt.Sprintf("node%d.input_stream.snd_url", i)
	mngk := fmt.Sprintf("node%d.input_stream.manage", i)
	prok := fmt.Sprintf("node%d.input_stream.protected", i)
	return design.SNDServerConfig{
		ServerAddrs: gh.v.GetString(urlk),
		Manage:      gh.v.GetStringSlice(mngk),
		Protected:   gh.v.GetStringSlice(prok),
	}
}
func (gh *graphHandler) readOutputStreamTo(i int) string {
	key := fmt.Sprintf("node%d.output_stream.to", i)
	return strings.ToLower((gh.v.GetString(key)))
}
func (gh *graphHandler) readOutputStreamDB(i int) design.DataBaseConfig {
	urlk := fmt.Sprintf("node%d.output_stream.db_url", i)
	namek := fmt.Sprintf("node%d.output_stream.db_name", i)
	tbk := fmt.Sprintf("node%d.output_stream.db_table", i)
	tagk := fmt.Sprintf("node%d.output_stream.db_tags", i)
	return design.DataBaseConfig{
		DBUrl:   gh.v.GetString(urlk),
		DBName:  gh.v.GetString(namek),
		DBTable: gh.v.GetString(tbk),
		Tags:    gh.v.GetStringSlice(tagk),
	}
}

func (gh *graphHandler) readOutputStreamFile(i int) design.FileConfig {
	key := fmt.Sprintf("node%d.output_stream.file", i)
	return design.FileConfig{FileName: gh.v.GetString(key)}
}
func (gh *graphHandler) readInputStreamDB(i int) design.DataBaseConfig {
	urlk := fmt.Sprintf("node%d.input_stream.db_url", i)
	namek := fmt.Sprintf("node%d.input_stream.db_name", i)
	tbk := fmt.Sprintf("node%d.input_stream.db_table", i)
	tagk := fmt.Sprintf("node%d.input_stream.db_tags", i)
	return design.DataBaseConfig{
		DBUrl:   gh.v.GetString(urlk),
		DBName:  gh.v.GetString(namek),
		DBTable: gh.v.GetString(tbk),
		Tags:    gh.v.GetStringSlice(tagk),
	}
}
func (gh *graphHandler) readOutputStreamNodes(i int) []string {
	key := fmt.Sprintf("node%d.output_stream.nodes", i)
	return gh.v.GetStringSlice(key)
}

/* 图的基本结构与解析 */
type InputStreamSpec struct {
	From      string // 与graph input一致，或者来源于nodes
	SNDServer design.SNDServerConfig
	DataBase  design.DataBaseConfig
	PcapFile  design.PcapFileConfig
	Nodes     []string
}

type OutputStreamSpec struct {
	To       string // 与graph output一致，或者to nodes
	DataBase design.DataBaseConfig
	File     design.FileConfig
	Nodes    []string
}

type NodeConfig struct {
	NodeName     string
	NodeID       int
	Runner       string
	InputStream  InputStreamSpec
	OutputStream OutputStreamSpec
}

type GraphConfig struct {
	handle   *graphHandler
	Input    string
	Output   string
	NodesNum int
	Nodes    []NodeConfig // key=nodeName, ex:node1,node2
	LinkMap  map[int][]int
}

func (gh *graphHandler) parseNodeConfig(graphInput, graphOutput string, nodeNum, i int) (NodeConfig, error) {
	node := NodeConfig{
		NodeName: fmt.Sprintf("node%d", i),
		NodeID:   i,
	}
	node.Runner = gh.readNodeRunner(i)
	if err := runnerIsValid(node.Runner); err != nil {
		return node, err
	}
	// 解析node input stream
	inputStream := InputStreamSpec{}
	inputStream.From = gh.readInputStreamFrom(i)
	if err := inputFromIsValid(graphInput, inputStream.From, i); err != nil {
		return node, err
	}

	switch inputStream.From {
	case design.INPUT_FROM_SND_SERVER_CONFIG:
		inputStream.SNDServer = gh.readInputStreamSNDConfig(i)
	case design.INPUT_FROM_DATABASE:
		inputStream.DataBase = gh.readInputStreamDB(i)
	case design.INPUT_FROM_PCAP_FILE:
		inputStream.PcapFile = gh.readInputStreamPcapFile(i)
	case design.INPUT_FROM_NODES:
		inputStream.Nodes = gh.readInputStreamNodes(i)
	}
	node.InputStream = inputStream

	// 解析node output stream
	outputStream := OutputStreamSpec{}
	outputStream.To = gh.readOutputStreamTo(i)
	if err := outputToIsValid(graphOutput, outputStream.To, nodeNum, i); err != nil {
		return node, err
	}
	switch outputStream.To {
	case design.OUTPUT_TO_FILE:
		outputStream.File = gh.readOutputStreamFile(i)
	case design.OUTPUT_TO_DATABASE:
		outputStream.DataBase = gh.readOutputStreamDB(i)
	case design.OUTPUT_TO_NODES:
		outputStream.Nodes = gh.readOutputStreamNodes(i)
	}
	node.OutputStream = outputStream
	return node, nil
}

func (gh *graphHandler) parseGraphConfig() (*GraphConfig, error) {
	gc := &GraphConfig{
		handle:  gh,
		LinkMap: make(map[int][]int),
	}
	gc.Input = gh.readGraphInputStream()
	//TODO: check graph input
	gc.Output = gh.readGraphOutputStream()
	//TODO: check graph output
	gc.NodesNum = gh.readGraphNodesNum()
	if gc.NodesNum == 0 {
		// TODO: 没有运行节点 --> 不运行任务还是报错
		return gc, nil
	}
	gc.Nodes = make([]NodeConfig, 0)
	for i := 1; i <= gc.NodesNum; i++ {
		node, err := gh.parseNodeConfig(gc.Input, gc.Output, gc.NodesNum, i)
		if err != nil {
			return nil, err
		}
		gc.Nodes = append(gc.Nodes, node)
	}
	return gc, nil
}

/*  验证图的结构合理性  */
func (gc *GraphConfig) valid() error {

	for i, node := range gc.Nodes {
		// 先检查node from的结构体是否为空
		switch node.InputStream.From {
		case design.INPUT_FROM_SND_SERVER_CONFIG:
			// check sndserverconfig结构体
		case design.INPUT_FROM_DATABASE:
			// check database结构体
		case design.INPUT_FROM_PCAP_FILE:
			// check pcapfile结构体
		case design.INPUT_FROM_NODES:
			if len(node.InputStream.Nodes) == 0 {
				// input 为nodes，但是nodes结构体为空
				return fmt.Errorf("node <%s> input stream from nodes, but nodes is empty", node.NodeName)
			}
			// nodes不为空，检查每个node的输出是不是这个node.NodeName
			for _, inN := range node.InputStream.Nodes {
				id, _ := strconv.ParseInt(strings.TrimLeft(inN, "node"), 10, 64)
				inNode := gc.Nodes[id-1]
				find := false
				for _, outNode := range inNode.OutputStream.Nodes {
					if outNode == node.NodeName {
						find = true
						break
					}
				}
				if !find {
					return fmt.Errorf("<%s> does not have output node <%s>", inN, node.NodeName)
				}
			}
		}

		// 再检查node to的结构是否为空
		switch node.OutputStream.To {
		case design.OUTPUT_TO_DATABASE:
			// check
		case design.OUTPUT_TO_FILE:
			// check
		case design.OUTPUT_TO_NODES:
			if len(node.OutputStream.Nodes) == 0 {
				return fmt.Errorf("<%s> output to nodes, but nodes is empty", node.NodeName)
			}
			// 检查每个output的input是否包含node.NodeName
			outInt := []int{}
			for _, outN := range node.OutputStream.Nodes {
				id, _ := strconv.ParseInt(strings.TrimLeft(outN, "node"), 10, 64)
				outNode := gc.Nodes[id-1]
				find := false
				for _, inNode := range outNode.InputStream.Nodes {
					if inNode == node.NodeName {
						find = true
						break
					}
				}
				if !find {
					return fmt.Errorf("<%s> does not have input node <%s>", outN, node.NodeName)
				}
				outInt = append(outInt, int(id))
			}
			gc.LinkMap[i+1] = outInt
		}
	}
	return nil
}
