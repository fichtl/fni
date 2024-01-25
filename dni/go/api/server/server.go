package server

import (
	"fmt"
	"net"

	"github.com/amianetworks/dni/api/dnipb"
	"google.golang.org/grpc"
	"google.golang.org/grpc/reflection"
)

const (
	defaultHostAddr string = "localhost:10502"
)

type DNIServer struct {
	host    string
	listen  net.Listener
	grpcsvc *grpc.Server
}

func NewDNIServer(host string) (*DNIServer, error) {
	server := &DNIServer{}
	if host == "" {
		host = defaultHostAddr
	}

	server.host = host
	server.grpcsvc = grpc.NewServer()
	dnipb.RegisterDNIServer(server.grpcsvc, server)

	reflection.Register(server.grpcsvc)
	lis, err := net.Listen("tcp", server.host)
	if err != nil {
		return nil, fmt.Errorf("failed to create listen (err=%v)", err)
	}
	server.listen = lis
	return server, nil
}

func (dni *DNIServer) Start() error {
	if err := dni.grpcsvc.Serve(dni.listen); err != nil {
		return fmt.Errorf("failed to start DNI Server (err=%v)", err)
	}
	return nil
}

func (dni *DNIServer) Stop() error {
	if dni.grpcsvc != nil {
		dni.grpcsvc.Stop()
	}
	return dni.listen.Close()
}
