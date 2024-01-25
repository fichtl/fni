package design

import "os"

const (
	// 图的输入类型有
	INPUT_FROM_SND_SERVER_CONFIG string = "sndserverconfig"
	INPUT_FROM_DATABASE          string = "database"
	INPUT_FROM_PCAP_FILE         string = "pcapfile"
	INPUT_FROM_NODES             string = "nodes"

	// 图的输出类型有
	OUTPUT_TO_DATABASE string = "database"
	OUTPUT_TO_FILE     string = "file"
	OUTPUT_TO_NODES    string = "nodes"
)

// 图不同的输入输出类型，对应的数据结构
// "sndServerConfig" 对应的数据结构如下
type SNDServerConfig struct {
	ServerAddrs string   // snd server的地址，ex. 10.1.17.2:8001
	Manage      []string // snd server的管理网卡
	Protected   []string // snd server的数据采集网卡
}

// "dataBase"对应的数据结构如下
type DataBaseConfig struct {
	DBUrl   string
	DBName  string
	DBTable string
	Tags    []string // 只在检索任务时使用
}

// "pcapFile"对应的数据结构如下
type PcapFileConfig struct {
	PcapFile string // pcap文件的绝对地址
}

// "file"对应的数据结构如下
type FileConfig struct {
	FileName   string // file的绝对地址
	FileHandle *os.File
}
