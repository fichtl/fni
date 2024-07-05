package generator

import (
	"bufio"
	"encoding/json"
	"fmt"
	"math"
	"os"
	"regexp"
	"strconv"
	"strings"

	"github.com/amianetworks/dni/sdk/task/dms/common"
	"github.com/google/gopacket"
	"github.com/google/gopacket/pcap"
	"github.com/shirou/gopsutil/cpu"
)

// CPU
var (
	all, soft float64
)

func GetCPU() (float64, error) {
	t, err := cpu.Times(false)
	if err != nil {
		return 0.0, err
	}
	lastAll, lastSoft := all, soft
	all, soft = GetSoftirq(t[0])
	return math.Min(100, math.Max(0, (soft-lastSoft)/(all-lastAll)*100)), nil
}

func GetSoftirq(t cpu.TimesStat) (float64, float64) {
	busy := t.User + t.System + t.Nice + t.Iowait + t.Irq + t.Softirq + t.Steal
	return busy + t.Idle, t.Softirq
}

// BW
func GetNetifStat(dev string) (map[string]uint64, error) {
	statMap := map[string]uint64{
		"rx_bytes": 0, "rx_packets": 0, "tx_bytes": 0, "tx_packets": 0,
	}
	NIC_STATS_PATH := "/sys/class/net/%s/statistics/%s"
	for statType := range statMap {
		cnt, err := ReadUint(fmt.Sprintf(NIC_STATS_PATH, dev, statType))
		if err != nil {
			return nil, err
		}
		statMap[statType] = cnt
	}

	return statMap, nil
}

func ReadUint(filePath string) (uint64, error) {
	str, err := ReadString(filePath)
	if err != nil {
		return 0, err
	}
	return strconv.ParseUint(str, 10, 64)
}

func ReadString(filePath string) (string, error) {
	bytes, err := os.ReadFile(filePath)
	if err != nil {
		return "", err
	}
	return strings.TrimRight(string(bytes), "\n"), nil
}

// TCPCONN
func GetTCPConnBriefProc() (map[string]uint64, error) {
	PROC_NET_TCP := "/proc/net/tcp"
	LO_IPV4_HEX := "0100007F"
	f, err := os.Open(PROC_NET_TCP)
	if err != nil {
		return nil, err
	}
	defer f.Close()

	connStats := make([]uint64, 12)
	s := bufio.NewScanner(f)
	s.Scan() // skip first line
	for s.Scan() {
		fields := strings.Fields(s.Text())
		state, err := strconv.ParseInt(fields[3], 16, 64)
		if err != nil {
			continue
		}
		ipp := strings.Split(fields[1], ":")
		// Don't count malformed field or connections that bind to localhost
		if len(ipp) != 2 || ipp[0] == LO_IPV4_HEX {
			continue
		}
		connStats[state]++
	}
	tcpConnStats := make(map[string]uint64)
	tcpConnStats["TCP_SYN_RECV"] = connStats[3]
	tcpConnStats["TCP_ESTABLISHED"] = connStats[1]
	tcpConnStats["TCP_TIME_WAIT"] = connStats[6]
	return tcpConnStats, err
}

// SNMP
var feats []string = []string{
	"TcpExtIPReversePathFilter",

	"IcmpInMsgs",

	"TcpOutRsts",
	"TcpInSegs",
	"TcpExtTCPReqQFullDrop",
	"TcpExtTCPReqQFullDoCookies",

	"UdpRcvbufErrors",
	"UdpNoPorts",
}

type snmpCounter struct {
	Kernel map[string]uint64 `json:"kernel"`
}

func GetSNMP() (map[string]uint64, error) {
	snmpCounter, err := getSnmpCounters(feats)
	if err != nil {
		return nil, nil
	}
	return snmpCounter.Kernel, nil
}

func getSnmpCounters(tokens []string) (*snmpCounter, error) {
	if len(tokens) == 0 {
		return nil, nil
	}
	out, err := common.RunShellCmdf("nstat -j %s", strings.Join(tokens, " "))
	if err != nil {
		return nil, err
	}
	c := &snmpCounter{}
	err = json.Unmarshal([]byte(out), c)
	if err != nil {
		return nil, err
	}
	return c, nil
}

// TCP Connection Infos
func GetTcpConnInfos() []*common.ConnInfo {
	reSpaces := regexp.MustCompile(`\s+`)
	SS_TCP_ALL_P := "ss -nHt4p state all"
	conns := make([]*common.ConnInfo, 0)
	for _, line := range common.CmdOutputLines(SS_TCP_ALL_P) {
		if strings.TrimSpace(line) == "" {
			continue
		}
		tokens := reSpaces.Split(line, 7)
		if tokens[0] == "ESTAB" {
			send, err := strconv.Atoi(tokens[1])
			if err != nil {
				continue
			}
			rscv, err := strconv.Atoi(tokens[2])
			if err != nil {
				continue
			}
			sip, sport, err := parseIPPortString(tokens[3])
			if err != nil {
				continue
			}
			dip, dport, err := parseIPPortString(tokens[4])
			if err != nil {
				continue
			}
			conn := new(common.ConnInfo)
			conn.Send = send
			conn.Recv = rscv
			conn.Src = sip
			conn.Dst = dip
			conn.SPort = sport
			conn.DPort = dport
			conn.Name = tokens[3] + "-->" + tokens[4]
			conns = append(conns, conn)
		}
	}
	return conns
}

func parseIPPortString(token string) (string, int, error) {
	if strings.HasPrefix(token, "[") {
		return "", 0, fmt.Errorf("IPv6 address not supported yet: %q", token)
	}
	s := strings.Split(token, ":")
	if len(s) != 2 {
		return "", 0, fmt.Errorf("malformed IP:Port: %q", token)
	}
	port, err := strconv.ParseUint(s[1], 10, 16)
	if err != nil {
		return "", 0, err
	}
	ip := s[0]
	// For SO_BINDTODEVICE socket that contains `%' in address
	if idx := strings.Index(ip, "%"); idx >= 0 {
		ip = ip[:idx]
	}
	return ip, int(port), nil
}

// Listen Port Infos
func GetListenInfos() []map[string]string {
	ssListen := "ss -nHtu -l"
	listenInfos := make([]map[string]string, 0)
	for _, line := range common.CmdOutputLines(ssListen) {
		if line == "" || strings.TrimSpace(line) == "" {
			continue
		}
		tokens := strings.Fields(line)
		if tokens[1] == "LISTEN" || tokens[1] == "UNCONN" {
			d := make(map[string]string)
			d["PROTO"] = tokens[0]
			d["STATUS"] = tokens[1]
			sip, sport, err := parseIPPortString(tokens[4])
			if err != nil {
				continue
			}
			d["SIP"] = sip
			d["SPORT"] = fmt.Sprintf("%d", sport)
			listenInfos = append(listenInfos, d)
		}
	}
	return listenInfos
}

// Conntraks
func GetCtInfoConntrack() ([]map[string]string, error) {
	conntrackDump := "sudo conntrack -L"
	out, err := common.CmdOutput(conntrackDump)
	if err != nil {
		return nil, err
	}
	s := bufio.NewScanner(strings.NewReader(out))
	conntracks := make([]map[string]string, 0)
	for s.Scan() {
		text := s.Text()
		if strings.TrimSpace(text) == "" {
			continue
		}
		fields := strings.Fields(text)
		conntrack := make(map[string]string)
		conntrack["Proto"] = fields[0]
		switch fields[0] {
		case "icmp":
			orig, reply, _, err := parseConntrack(fields, 3, 5)
			if err != nil {
				continue
			}
			conntrack["oSIP"] = orig["src"]
			conntrack["oDIP"] = orig["dst"]
			conntrack["rSIP"] = reply["src"]
			conntrack["rDIP"] = reply["dst"]
			conntrack["flag"] = ""
			conntrack["status"] = ""
			conntracks = append(conntracks, conntrack)
		case "tcp":
			orig, reply, flag, err := parseConntrack(fields, 3, 4)
			if err != nil {
				continue
			}
			conntrack["oSIP"] = orig["src"]
			conntrack["oDIP"] = orig["dst"]
			conntrack["rSIP"] = reply["src"]
			conntrack["rDIP"] = reply["dst"]
			conntrack["flag"] = flag
			conntrack["status"] = fields[3]
			conntracks = append(conntracks, conntrack)
		}

	}
	return conntracks, nil
}

func parseConntrack(fields []string, start, step int) (map[string]string, map[string]string, string, error) {
	var (
		orig  = make(map[string]string)
		reply = make(map[string]string)
		flag  string
	)

	i := start
	end := i + step
	if len(fields) <= end {
		return nil, nil, "", fmt.Errorf("malformed ct fields: %s", fields)
	}
	for ; i < end; i++ {
		kv := strings.Split(fields[i], "=")
		if len(kv) != 2 {
			continue
		}
		//k:sip v:1.2.3.4
		orig[kv[0]] = kv[1]
	}
	if fields[i][0] == '[' {
		flag = fields[i]
		i++
	}
	end = i + step
	if len(fields) <= end {
		return nil, nil, "", fmt.Errorf("malformed ct fields: %s", fields)
	}
	for ; i < end; i++ {
		kv := strings.Split(fields[i], "=")
		if len(kv) != 2 {
			continue
		}
		reply[kv[0]] = kv[1]
	}
	if fields[i][0] == '[' {
		flag = fields[i]
	}
	return orig, reply, flag, nil
}

// parsepcap
func GetPackets(fin string) ([]gopacket.Packet, error) {
	handle, err := pcap.OpenOffline(fin)
	if err != nil {
		return nil, err
	}
	packetSource := gopacket.NewPacketSource(handle, handle.LinkType())
	packets := make([]gopacket.Packet, 0)
	for p := range packetSource.Packets() {
		packets = append(packets, p)
	}
	return packets, nil
}
