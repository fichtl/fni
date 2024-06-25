package analyzer

import (
	"github.com/amianetworks/dni/sdk/task/dms/common"
	config "github.com/amianetworks/dni/sdk/task/dms/config"
)

func SlowHttpDetection(conndiag *ConnDiag, connTh config.TcpConnUtilThreshold, ipNic map[string]string) *Respond {
	if conndiag == nil {
		return nil
	}
	if est := conndiag.Estab; est != nil {
		if est.Count >= connTh.FullConn {
			resp := newRespond()
			diagFullConnAttack(&resp.AttackInfo, est, connTh, ipNic)
			return resp
		}
	}
	return nil
}

type pattern struct {
	Host string
	send int
	recv int
}

func filterCiBySendRecvQ(all []*common.ConnInfo, th int) map[pattern][]*common.ConnInfo {
	agg := make(map[pattern][]*common.ConnInfo)

	var sr pattern
	for _, ci := range all {
		// ignore all zero cases
		if ci.Send == 0 && ci.Recv == 0 {
			continue
		}
		sr.Host = ci.Src
		sr.send = ci.Send
		sr.recv = ci.Recv
		agg[sr] = append(agg[sr], ci)
	}
	for k, cis := range agg {
		if len(cis) <= th {
			delete(agg, k)
		}
	}
	return agg
}

// diagFullConnAttack check per-port TCP connestions
func diagFullConnAttack(a *AttackInfo, stats *ConnStat, connTh config.TcpConnUtilThreshold, ipNic map[string]string) {
	var conns []*common.ConnInfo

	rs := common.NewRuleSet()

	// thpps := config.GetPPSThreshold(common.PROTO_STR_TCP, false)
	// thctime := config.GetConnCtimeThreshold()
	thportconn := connTh.PerPortFullConn
	// TODO: PerPort ConnInfo
	for _, stat := range stats.Flows {
		if stat.Count < thportconn {
			goto end // sorted, fastpath
		}
		a.Type[ATTACK_TCP_FULL] = 1

		port := stat.Key.(common.PORT)

		// Slow read
		long := filterCiBySendRecvQ(stats.PerPort[port], thportconn)
		if len(long) != 0 {
			a.Type[ATTACK_HTTP_SLOW_READ] = 1
			for k, cis := range long {
				// set mitigation ifname by the attacked local IP address.
				a.Dev = LookupDev(cis[0].Dst, ipNic)

				// only record the first conninfo since they have same host
				conns = append(conns, cis[0])
				// common.DMSLogger.Debug.Println(cis)

				// TODO: check number of host
				rs.Add(genL3l4Rule(k.Host, common.PROTO_STR_TCP, port, false, 0, 0))

				// processes = append(processes, ps...)
			}
		}

		// Otherwise, we should limit traffic to this port
		// rs.Add(genPortLimitRule("tcp", port, false, common.ACTION_LIMIT_PPS, uint64(thpps)))
	}
end:
	a.Pattern = rs
	if len(conns) != 0 {
		a.TCPConn = conns
	}
}
