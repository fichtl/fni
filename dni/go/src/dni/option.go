package main

import (
	"github.com/amianetworks/am.modules/log"
	"github.com/spf13/pflag"
)

const (
	cfgDefaultHostAddr string = "localhost:10502"
)

type dniConf struct {
	dniHost string
}

func newLogConf() log.LoggerConfig {
	lg := log.LoggerConfig{
		LogPrefix:        "[DNI]",
		MaxLogFileSize:   1024 * 10,
		MaxRotateFileNum: 5,
		Aconf: log.AlogConf{
			LogFile:      "/usr/local/dni/dni.log",
			LogLevel:     log.LOG_LEVEL_DEBUG,
			ReportCaller: true,
			LogFormat:    log.LOGGER_TYPE_STANDARD,
		},
		Pconf: log.PlogConf{
			LogFile:      "/usr/local/dni/dni.log",
			LogLevel:     log.LOG_LEVEL_DEBUG,
			ReportCaller: true,
			LogFormat:    log.LOGGER_TYPE_STANDARD,
		},
		Econf: log.ElogConf{
			LogFile:      "/usr/local/dni/dni.log",
			LogLevel:     log.LOG_LEVEL_DEBUG,
			ReportCaller: true,
		},
		Rconf: log.RlogConf{
			LogFile:       "/usr/local/dni/dni.log",
			FileLogLevel:  log.LOG_LEVEL_DEBUG,
			StdLogLevel:   log.LOG_LEVEL_ERROR,
			ReportCaller:  true,
			FileLogFormat: log.LOGGER_TYPE_STANDARD,
			StdLogFormat:  log.LOGGER_TYPE_STANDARD,
		},
	}
	return lg
}

func initDNIConf() *dniConf {
	return &dniConf{
		dniHost: cfgDefaultHostAddr,
	}
}

func (dc *dniConf) AddFlags(fs *pflag.FlagSet) {
	fs.StringVar(&dc.dniHost, "host-addr", dc.dniHost, "DNI host addr")
}
