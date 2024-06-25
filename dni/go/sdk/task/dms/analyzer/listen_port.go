package analyzer

import (
	"net"
	"strconv"

	"github.com/amianetworks/dni/sdk/task/dms/common"
)

func GetListenPort(protodiag *ProtoDiag, data []map[string]string, filter map[string]struct{}) {
	for _, d := range data {
		sip := d["SIP"]
		status := d["STATUS"]
		proto := d["PROTO"]
		if sip != "0.0.0.0" && sip != "*" {
			if lip := net.ParseIP(d["SIP"]); lip == nil || lip.IsLoopback() {
				continue
			}

			if _, ok := filter[sip]; !ok {
				continue
			}
		}
		switch status {
		case "LISTEN", "UNCONN":
			port, err := strconv.Atoi(d["SPORT"])
			if err != nil {
				continue
			}
			if proto == "tcp" {
				protodiag.Tcp.Listen[common.PORT(port)] = struct{}{}
			} else if proto == "udp" {
				protodiag.Udp.Listen[common.PORT(port)] = struct{}{}
			}
		}
	}
}
