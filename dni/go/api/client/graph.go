package client

import (
	"context"
	"fmt"
	"time"

	"github.com/amianetworks/dni/api/dnipb"
	"google.golang.org/grpc"
)

func RunGraph(graphFile string) (string, error) {
	conn, err := grpc.Dial(globalHostAddr, grpc.WithInsecure())
	if err != nil {
		return "", fmt.Errorf("failed to connect DNI Server(err=%v)", err)
	}
	defer conn.Close()

	c := dnipb.NewDNIClient(conn)

	ctx, cancel := context.WithTimeout(context.Background(), 10*time.Second)
	defer cancel()

	req := &dnipb.GraphReq{
		Request: graphFile,
	}
	reply, err := c.RunGraph(ctx, req)
	if err != nil {
		return "", fmt.Errorf("run graph failed, err=%v", err)
	}
	return reply.Content, nil
}

func PauseGraph(graphID string) error {
	conn, err := grpc.Dial(globalHostAddr, grpc.WithInsecure())
	if err != nil {
		return fmt.Errorf("failed to connect DNI Server(err=%v)", err)
	}
	defer conn.Close()

	c := dnipb.NewDNIClient(conn)

	ctx, cancel := context.WithTimeout(context.Background(), 10*time.Second)
	defer cancel()

	req := &dnipb.GraphReq{
		Request: graphID,
	}
	if _, err := c.PauseGraph(ctx, req); err != nil {
		return fmt.Errorf("pause graph failed, err=%v", err)
	}
	return nil
}

func RerunGraph(graphID string) error {
	conn, err := grpc.Dial(globalHostAddr, grpc.WithInsecure())
	if err != nil {
		return fmt.Errorf("failed to connect DNI Server(err=%v)", err)
	}
	defer conn.Close()

	c := dnipb.NewDNIClient(conn)

	ctx, cancel := context.WithTimeout(context.Background(), 10*time.Second)
	defer cancel()

	req := &dnipb.GraphReq{
		Request: graphID,
	}
	if _, err := c.RerunGraph(ctx, req); err != nil {
		return fmt.Errorf("rerun graph failed, err=%v", err)
	}
	return nil
}

func DestroyGraph(graphID string) error {
	conn, err := grpc.Dial(globalHostAddr, grpc.WithInsecure())
	if err != nil {
		return fmt.Errorf("failed to connect DNI Server(err=%v)", err)
	}
	defer conn.Close()

	c := dnipb.NewDNIClient(conn)

	ctx, cancel := context.WithTimeout(context.Background(), 10*time.Second)
	defer cancel()

	req := &dnipb.GraphReq{
		Request: graphID,
	}
	if _, err := c.DestroyGraph(ctx, req); err != nil {
		return fmt.Errorf("destroy graph failed, err=%v", err)
	}
	return nil
}
