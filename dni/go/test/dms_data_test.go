package test

import (
	"log"
	"testing"

	dms "github.com/amianetworks/dni/service/generator"
)

func TestGetTCPConn(t *testing.T) {
	res, err := dms.GetTCPConnBriefProc()
	if err != nil {
		t.Log(err)
	}
	t.Log(res)
}

func TestGetSNMP(t *testing.T) {
	res, err := dms.GetSNMP()
	if err != nil {
		t.Log(err)
	}
	t.Log(res)
}

func TestNicSpeed(t *testing.T) {
	nic_speed := 10_000
	log.Println(nic_speed)
}

func TestGetConntracks(t *testing.T) {
	conntracks, err := dms.GetCtInfoConntrack()
	if err != nil {
		t.Log(err)
	}
	t.Log(conntracks)
}
