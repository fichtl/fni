package main

import (
	lg "log"
	"os"
	"os/signal"
	"sync"
	"syscall"

	"github.com/amianetworks/am.modules/log"

	svc "github.com/amianetworks/dni/api/server"
	"github.com/spf13/pflag"
)

type DNISpec struct {
	wg     *sync.WaitGroup
	conf   *dniConf
	server *svc.DNIServer
}

var dni DNISpec

func initDNI() error {
	var err error

	dni.wg = &sync.WaitGroup{}
	dni.conf = initDNIConf()
	dni.conf.AddFlags(pflag.CommandLine)
	pflag.Parse()

	// init DNI log file
	logConf := newLogConf()
	if err = log.Init(logConf); err != nil {
		lg.Fatalf("init log failed, err=%v", err)
	}
	log.R.Info("success to init DNI log")

	dni.server, err = svc.NewDNIServer(dni.conf.dniHost)
	if err != nil {
		log.R.Errorf("create DNI Server failed, err=%v", err)
		goto __ERR_SERVER
	}

	go func() {
		if err = dni.server.Start(); err != nil {
			log.R.Errorf("start DNI Server failed, err=%v", err)
		}
	}()

	return nil
__ERR_SERVER:
	log.Finish()
	return err
}

func dniFinish() {
	log.Finish()
}

func main() {
	if err := initDNI(); err != nil {
		lg.Fatal(err)
	}
	log.R.Info("succeed to start DNI server")

	ch := make(chan os.Signal, 1)
	signal.Notify(ch, syscall.SIGINT, syscall.SIGTERM)
	dni.wg.Add(1)

	go func() {
		<-ch
		dniFinish()
		dni.wg.Done()
	}()

	dni.wg.Wait()
	log.R.Info("succeed to stop DNI Server")
	os.Exit(0)
}
