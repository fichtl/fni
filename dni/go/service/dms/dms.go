package main

import (
	"fmt"
	"time"

	alog "github.com/amianetworks/am.modules/log"

	_ "github.com/amianetworks/dni/sdk/task/dms/abitrator"
	_ "github.com/amianetworks/dni/sdk/task/dms/assessor"
	"github.com/amianetworks/dni/service/config"
	"github.com/amianetworks/dni/service/generator"
	"github.com/amianetworks/dni/service/manager"
	_ "github.com/amianetworks/dni/service/manager"
)

type DMS struct {
	rb     *generator.ShareMemomry
	conf   *config.DMSConfig
	graphs []string
}

var dms DMS

func newLogConf() alog.LoggerConfig {
	logConf := alog.LoggerConfig{
		LogPrefix:        "DNI-DMS",
		MaxLogFileSize:   1024 * 1024 * 1024,
		MaxRotateFileNum: 5,
		Aconf: alog.AlogConf{
			LogFile:      "/usr/local/dni/dni-dms.log",
			LogLevel:     alog.LoggerLevelDebug,
			ReportCaller: true,
			LogFormat:    alog.LoggerTypeStandard,
		},
		Pconf: alog.PlogConf{
			LogFile:      "/usr/local/dni/dni-dms.log",
			LogLevel:     alog.LoggerLevelDebug,
			ReportCaller: true,
			LogFormat:    alog.LoggerTypeStandard,
		},
		Econf: alog.ElogConf{
			LogFile:      "/usr/local/dni/dni-dms.log",
			LogLevel:     alog.LoggerLevelDebug,
			ReportCaller: true,
		},
		Rconf: alog.RlogConf{
			LogFile:       "/usr/local/dni/dni-dms.log",
			FileLogLevel:  alog.LoggerLevelDebug,
			StdLogLevel:   alog.LoggerLevelError,
			ReportCaller:  false,
			FileLogFormat: alog.LoggerTypeStandard,
			StdLogFormat:  alog.LoggerTypeStandard,
		},
	}
	return logConf
}

func initDMS() error {
	var err error
	//init log
	if err := alog.Init(newLogConf()); err != nil {
		fmt.Printf("init log error:%v", err)
		return err
	}
	//init dms config
	path := "dms.conf"
	dmsConf, err := config.GetDMSConfig(path)
	if err != nil {
		// log.Printf("get dms config failed, err=%v", err)
		alog.R.Errorf("get dms config failed, err=%v", err)
		return err
	}
	dms.conf = dmsConf
	//initialize dms graphs
	gNames := make([]string, 0)
	for _, g := range dmsConf.Graph {
		sidedataMap := make(map[string]interface{})
		gName, err := manager.RunGraph(g.Name, g.Path, sidedataMap)
		if err != nil {
			//log.Printf("run graph(%s) failed, err=%v", g.Name, err)
			alog.R.Errorf("run graph(%s) failed, err=%v", g.Name, err)
			return err
		}
		gNames = append(gNames, gName)
	}
	dms.graphs = gNames
	//initialize ringbuffer
	rb, err := generator.NewShareMemomry(dmsConf.Shm.ShmKey, dmsConf.Shm.MemorySize)
	if err != nil {
		//log.Printf("cretae share memory failed:%v", err)
		alog.R.Errorf("cretae share memory failed:%v", err)
		return err
	}
	dms.rb = rb
	return nil
}

func main() {
	if err := initDMS(); err != nil {
		return
	}
	//log.Println("succeed to start DNI-DMS")
	alog.R.Info("Succeed to start DNI-DMS")
	//initialize generator
	id := 0
	gNum := len(dms.graphs)
	for {
		dataptr, ok := dms.rb.Read()
		if !ok {
			time.Sleep(100 * time.Millisecond)
			continue
		}
		//generate input data
		nicip, assessorData, ratio, packets := generator.RingBufferDataParse(dataptr)
		listen := generator.GetListenInfos()
		tcpconn := generator.GetTcpConnInfos()
		inputdata := make(map[string]interface{})
		inputdata["assessorData"] = assessorData
		inputdata["listen"] = listen
		inputdata["tcpconn"] = tcpconn
		inputdata["packet"] = packets
		inputdata["ratio"] = ratio
		inputdata["nicIP"] = nicip
		calculateStartTime := time.Now()
		//add input stream to graph
		graphID := id % gNum
		id++
		for dev := range nicip {
			alog.R.Infof("Detecting dev %s", dev)
		}
		manager.AddInputsToGraph(dms.graphs[graphID], inputdata)
		_, err := manager.GetGraphOutputs(dms.graphs[graphID])
		if err != nil {
			alog.R.Errorf("get results error:%v", err)
		}
		endtime := time.Now()
		duration := endtime.Sub(calculateStartTime)
		alog.R.Infof("Detect duration:%v", duration)
	}
}
