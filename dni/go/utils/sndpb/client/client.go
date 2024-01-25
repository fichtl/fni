package client

import (
	"context"
	"fmt"
	"time"

	"github.com/amianetworks/dni/utils/sndpb"
	"google.golang.org/grpc"
)

func Ping(hostAddr string) error {

	conn, err := grpc.Dial(hostAddr, grpc.WithInsecure())
	if err != nil {
		return fmt.Errorf("failed to connect SNDing Server(err=%v)", err)
	}
	defer conn.Close()

	c := sndpb.NewSndingClient(conn)

	ctx, cancel := context.WithTimeout(context.Background(), 20*time.Second)
	defer cancel()

	req := &sndpb.Empty{}
	if _, err := c.Ping(ctx, req); err != nil {
		return fmt.Errorf("failed to ping SND service, err: %v", err)
	}
	return nil
}

func SetNetifs(hostAddr string, protected, manage []byte) error {

	conn, err := grpc.Dial(hostAddr, grpc.WithInsecure())
	if err != nil {
		return fmt.Errorf("failed to connect SNDing Server(err=%v)", err)
	}
	defer conn.Close()

	c := sndpb.NewSndingClient(conn)

	ctx, cancel := context.WithTimeout(context.Background(), 20*time.Second)
	defer cancel()

	req := &sndpb.NetifsReq{
		Format:    "name",
		Protected: protected,
		Manage:    manage,
	}
	if _, err := c.SetNetifs(ctx, req); err != nil {
		return fmt.Errorf("failed to set network interfaces, err: %v", err)
	}
	return nil
}

func SetDatabaseUrl(hostAddr, url string) error {
	conn, err := grpc.Dial(hostAddr, grpc.WithInsecure())
	if err != nil {
		return fmt.Errorf("failed to connect SNDing Server(err=%v)", err)
	}
	defer conn.Close()

	c := sndpb.NewSndingClient(conn)

	ctx, cancel := context.WithTimeout(context.Background(), 20*time.Second)
	defer cancel()

	req := &sndpb.DatabaseReq{
		Url: url,
	}
	if _, err := c.SetDatabaseUrl(ctx, req); err != nil {
		return fmt.Errorf("failed to set database url, err: %v", err)
	}
	return nil
}

func StartCollection(hostAddr string) error {
	conn, err := grpc.Dial(hostAddr, grpc.WithInsecure())
	if err != nil {
		return fmt.Errorf("failed to connect SNDing Server(err=%v)", err)
	}
	defer conn.Close()

	c := sndpb.NewSndingClient(conn)

	ctx, cancel := context.WithTimeout(context.Background(), 20*time.Second)
	defer cancel()

	req := &sndpb.Empty{}

	if _, err := c.StartCollection(ctx, req); err != nil {
		return fmt.Errorf("failed to start collection(err=%v)", err)
	}
	return nil
}

func StopCollection(hostAddr string) error {
	conn, err := grpc.Dial(hostAddr, grpc.WithInsecure())
	if err != nil {
		return fmt.Errorf("failed to connect SNDing Server(err=%v)", err)
	}
	defer conn.Close()

	c := sndpb.NewSndingClient(conn)

	ctx, cancel := context.WithTimeout(context.Background(), 20*time.Second)
	defer cancel()

	req := &sndpb.Empty{}
	if _, err := c.StopCollection(ctx, req); err != nil {
		return fmt.Errorf("failed to stop collection(err=%v)", err)
	}
	return nil
}

func SetHostID(hostAddr, id string) error {
	conn, err := grpc.Dial(hostAddr, grpc.WithInsecure())
	if err != nil {
		return fmt.Errorf("failed to connect SNDing Server(err=%v)", err)
	}
	defer conn.Close()

	c := sndpb.NewSndingClient(conn)

	ctx, cancel := context.WithTimeout(context.Background(), 20*time.Second)
	defer cancel()

	req := &sndpb.HostReq{
		ID: id,
	}
	if _, err := c.SetHostID(ctx, req); err != nil {
		return fmt.Errorf("failed to set host ID(err=%v)", err)
	}
	return nil
}
