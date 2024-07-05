package analyzer

import "github.com/amianetworks/dni/sdk/task/dms/common"

type ConnDiag struct {
	Estab *ConnStat
}

func ConnAnalyser(msmt map[Measurement]struct{}, conns []*common.ConnInfo) *ConnDiag {
	var (
		states = make(map[string]map[uint8]struct{})
	)
	//query.Conn
	for m := range msmt {
		// semi-conn attacks
		if m.L2 == CONN_ALL || m.L2 == CONN_TCP_SYN {
			states["SEMI"] = map[uint8]struct{}{
				common.TCP_SYN_RECV: {},
			}
		}
		// slow attacks
		if m.L2 == CONN_ALL || m.L2 == CONN_TCP_ESTABLISHED {
			states["ESTAB"] = map[uint8]struct{}{
				common.TCP_ESTABLISHED: {},
			}
		}
	}
	ss := tcpConnAnalyser(states, conns)

	conndiag := &ConnDiag{
		Estab: ss,
	}
	return conndiag
}

func tcpConnAnalyser(states map[string]map[uint8]struct{}, conns []*common.ConnInfo) *ConnStat {
	if len(states) == 0 {
		return nil
	}
	//get ci(per port conn info)
	ci := make(common.PortConnInfo)
	flowset := make(common.FlowSet)
	for _, conn := range conns {
		f := conn.Flow()
		clist, ok := ci[f.Rport]
		if !ok {
			clist = make([]*common.ConnInfo, 0)
		}
		ci[f.Rport] = append(clist, conn)
		flowset[f] = struct{}{}
	}

	var connStats *ConnStat

	// get per-remote flowset for each state
	n, fs := groupByLport(flowset)
	connStats = new(ConnStat)
	connStats.Count = n
	connStats.Flows = fs
	connStats.PerPort = ci

	return connStats
}

func groupByLport(flowset common.FlowSet) (int, []common.FlowStat) {
	var stats = make(common.FlowStatMap)
	for flow := range flowset {
		key := flow.Lport
		if _, ok := stats[key]; !ok {
			stats[key] = make(common.FlowSet)
		}
		stats[key][flow] = struct{}{}
	}
	return len(flowset), common.SortFlowStatMap(stats)
}

func GetCtInfoSS(ct *common.CtInfo, conns []*common.ConnInfo, filter map[string]struct{}) {
	for _, tcpconn := range conns {
		if _, ok := filter[tcpconn.Src]; !ok || tcpconn.Src == tcpconn.Dst {
			continue
		}
		ct.Est[tcpconn.Dst] = struct{}{}
	}
}
