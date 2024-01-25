package node

import (
	"encoding/json"

	"github.com/amianetworks/am.modules/log"
	"github.com/amianetworks/dni/src/data"
	"github.com/amianetworks/dni/src/design"
	"github.com/amianetworks/dni/src/graph"
	"github.com/amianetworks/dni/utils/sndpb/client"
)

type TelemetryInputHandler struct {
	SNDConf design.SNDServerConfig
}

type TelemetryOutputHandler struct {
	SNDSvcAddr string
	DB         design.DataBaseConfig
}

// input and output 初始化接口
func InitializeTelemetryInputHandler(nc graph.NodeConfig) TelemetryInputHandler {
	inputHandler := TelemetryInputHandler{
		SNDConf: nc.InputStream.SNDServer,
	}
	return inputHandler
}

func InitializeTelemetryOutputHandler(nc graph.NodeConfig) TelemetryOutputHandler {
	outputHandler := TelemetryOutputHandler{
		SNDSvcAddr: nc.InputStream.SNDServer.ServerAddrs,
		DB:         nc.OutputStream.DataBase,
	}
	return outputHandler
}

// input handler 的对应接口
func (tih *TelemetryInputHandler) ReceiveInputStreams(inputChan *data.InputManager) error {
	input := data.DataSpec{
		Type: data.DATA_TYPE_SND_CONFIG,
		Data: tih.SNDConf.ServerAddrs,
	}

	inputChan.AddPacket(input)
	return nil
}

func (tih *TelemetryInputHandler) PrepareForRun() error {
	mng, _ := json.Marshal(tih.SNDConf.Manage)
	pro, _ := json.Marshal(tih.SNDConf.Protected)
	if err := client.SetNetifs(tih.SNDConf.ServerAddrs, pro, mng); err != nil {
		return err
	}
	log.R.Info("[telemtry] configuration netifs")
	return nil
}

func (tih *TelemetryInputHandler) Close() error {
	// 销毁inputHandler的资源
	// telemtry 任务没有什么可销毁的
	return nil
}

// output handler 的对应接口
func (toh *TelemetryOutputHandler) PrepareForRun() error {
	if err := client.SetDatabaseUrl(toh.SNDSvcAddr, toh.DB.DBUrl); err != nil {
		return err
	}
	log.R.Info("[telemtry] configuration database")
	return nil
}

func (toh *TelemetryOutputHandler) SendOutputStreams(data data.DataSpec) error {
	return nil
}

func (toh *TelemetryOutputHandler) Close() error {
	// telemtry任务 output 不需要销毁DB的client
	// 因为不会初始化client
	return nil
}

// 任务执行接口
func TelemetryStartExecutor(dataArr []data.DataSpec, options data.DataSpec) (data.DataSpec, error) {

	if dataArr == nil {
		return data.DataSpec{Type: data.DATA_TYPE_NONE}, nil
	}

	for _, input := range dataArr {
		switch input.Type {
		case data.DATA_TYPE_SND_CONFIG:
			sndurl := input.Data.(string)
			if err := client.StartCollection(sndurl); err != nil {
				return data.DataSpec{}, err
			}
		}
	}
	log.R.Info("[telemetry] start telemetry")
	return data.DataSpec{Type: data.DATA_TYPE_NONE}, nil
}

func TelemetryStopExecutor(input data.DataSpec) error {
	switch input.Type {
	case data.DATA_TYPE_SND_CONFIG:
		sndurl := input.Data.(string)
		if err := client.StopCollection(sndurl); err != nil {
			return err
		}
	}
	log.R.Info("[telemetry] stop telemetry")
	return nil
}
