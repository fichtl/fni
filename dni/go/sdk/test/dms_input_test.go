package test

import (
	"log"
	"testing"
	"time"

	dms "github.com/amianetworks/dni/sdk/task/dms/data"
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

func TestGetAssessData(t *testing.T) {
	devs := []string{"eno1"}
	ag := dms.NewAssessorDataGenerator(devs)
	for i := 0; i < 5; i++ {
		data, err := ag.GetAssessData()
		if err != nil {
			t.Log(err)
		}
		t.Log(data)
		time.Sleep(time.Second)
	}
}

func TestGetConntracks(t *testing.T) {
	conntracks, err := dms.GetCtInfoConntrack()
	if err != nil {
		t.Log(err)
	}
	t.Log(conntracks)
}
