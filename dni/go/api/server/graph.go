package server

import (
	"context"

	"github.com/amianetworks/dni/api/dnipb"
	"github.com/amianetworks/dni/src/scheduler"
)

func (dni *DNIServer) RunGraph(ctx context.Context, req *dnipb.GraphReq) (*dnipb.GeneralReply, error) {
	graphID, err := scheduler.RunGraph(req.Request)
	if err != nil {
		return &dnipb.GeneralReply{Result: false, Content: err.Error()}, err
	}
	return &dnipb.GeneralReply{Result: true, Content: graphID}, nil
}

func (dni *DNIServer) PauseGraph(ctx context.Context, req *dnipb.GraphReq) (*dnipb.GeneralReply, error) {
	if err := scheduler.PauseGraph(req.Request); err != nil {
		return &dnipb.GeneralReply{Result: false, Content: err.Error()}, err
	}
	return &dnipb.GeneralReply{Result: true, Content: "success"}, nil
}

func (dni *DNIServer) RerunGraph(ctx context.Context, req *dnipb.GraphReq) (*dnipb.GeneralReply, error) {
	if err := scheduler.RerunGraph(req.Request); err != nil {
		return &dnipb.GeneralReply{Result: false, Content: err.Error()}, err
	}
	return &dnipb.GeneralReply{Result: true, Content: "success"}, nil
}

func (dni *DNIServer) DestroyGraph(ctx context.Context, req *dnipb.GraphReq) (*dnipb.GeneralReply, error) {
	if err := scheduler.DestroyGraph(req.Request); err != nil {
		return &dnipb.GeneralReply{Result: false, Content: err.Error()}, err
	}
	return &dnipb.GeneralReply{Result: true, Content: "success"}, nil
}
