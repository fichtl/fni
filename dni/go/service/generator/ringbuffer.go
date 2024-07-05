package generator

import (
	"fmt"
	"net"
	"strings"
	"syscall"
	"unsafe"

	"github.com/amianetworks/dni/sdk/task/dms/assessor"
	"github.com/google/gopacket"
	"github.com/google/gopacket/layers"
)

const SlotSize = 20 * 1024 * 1024

type v [SlotSize]byte

type Pointer struct {
	head uint32
	tail uint32
	size uint
}

type ShareMemomry struct {
	ShmKey      int
	MemorySize  int
	shmAddr     uintptr
	dataShmAddr uintptr
	pointer     *Pointer
	count       int
	offset      int
}

func NewShareMemomry(shmKey int, memorySize int) (*ShareMemomry, error) {
	sm := &ShareMemomry{
		ShmKey:     shmKey,
		MemorySize: memorySize,
	}
	//get share memory size
	size := sm.MemorySize + int(unsafe.Sizeof(*sm.pointer))
	//slot count
	sm.count = sm.MemorySize / SlotSize
	sm.offset = 0

	shmid, _, err := syscall.Syscall(syscall.SYS_SHMGET, uintptr(sm.ShmKey), uintptr(size), 00001000|0666)
	if err != 0 {
		return nil, fmt.Errorf("shmget failed: %v", err)
	}

	shmaddr, _, err := syscall.Syscall(syscall.SYS_SHMAT, shmid, 0, 0)
	if err != 0 {
		return nil, fmt.Errorf("shmat failed: %v", err)
	}

	sm.shmAddr = shmaddr
	sm.dataShmAddr = shmaddr + unsafe.Sizeof(*sm.pointer)

	sm.pointer = (*Pointer)(unsafe.Pointer(shmaddr))
	sm.pointer.head = sm.pointer.tail
	sm.pointer.size = uint(sm.MemorySize)
	return sm, nil
}

func (sm *ShareMemomry) Write(data []byte) {
	fmt.Println("trail ptr:", sm.pointer.tail)
	fmt.Println("write data:", data)
	sliceptr := (*v)(unsafe.Pointer(sm.dataShmAddr + uintptr(sm.pointer.tail)))
	copy((*sliceptr)[:], data)
	sm.pointer.tail = (sm.pointer.tail + SlotSize) % uint32(sm.pointer.size)
}

func (sm *ShareMemomry) Read() (uintptr, bool) {
	used := (sm.pointer.tail - sm.pointer.head + uint32(sm.pointer.size)) % uint32(sm.pointer.size)
	if used < SlotSize {
		return 0, false
	}
	dataptr := sm.dataShmAddr + uintptr(sm.pointer.head)
	sm.pointer.head = (sm.pointer.head + SlotSize) % uint32(sm.pointer.size)
	return dataptr, true
}

func RingBufferDataParse(dataPtr uintptr) (map[string]string, assessor.AssessorData, map[string]float64, map[string][]gopacket.Packet) {
	//res
	nicIP := make(map[string]string)
	assessorData := assessor.AssessorData{
		BW:       make(map[string]map[string]uint64),
		NicSpeed: make(map[string]uint64),
	}
	ratioMap := make(map[string]float64)
	packets := make(map[string][]gopacket.Packet)
	//offset
	offset := 0
	dataSlice := (*(*v)(unsafe.Pointer(dataPtr)))[:]
	//hostnic name length
	nameLength := *(*uint16)(unsafe.Pointer(dataPtr + uintptr(offset)))
	offset += 2
	//hostnic name
	hostNicName := string(dataSlice[offset : offset+int(nameLength)])
	nicName := strings.Split(hostNicName, "#")[1]
	offset += int(nameLength)
	//nic ip
	ipBytes := dataSlice[offset : offset+4]
	ipStr := net.IP(ipBytes).String()
	nicIP[nicName] = ipStr
	offset += 4
	//nic speed
	nicSpeed := *(*uint64)(unsafe.Pointer(dataPtr + uintptr(offset)))
	assessorData.NicSpeed[nicName] = nicSpeed
	offset += 8
	//get time stamp
	// ts := *(*uint64)(unsafe.Pointer(dataPtr + uintptr(offset)))
	offset += 8
	//get ratio
	ratio := *(*uint64)(unsafe.Pointer(dataPtr + uintptr(offset)))
	ratioMap[nicName] = float64(ratio) / 1000.0
	offset += 8
	//packet num
	countTotal := *(*uint32)(unsafe.Pointer(dataPtr + uintptr(offset)))
	offset += 4
	//statistics
	cpu := *(*float64)(unsafe.Pointer(dataPtr + uintptr(offset)))
	assessorData.CPU = cpu
	offset += 8 //cpu
	//bw
	bwMap := make(map[string]uint64)
	bwKeys := []string{"rx_bytes", "rx_packets", "tx_bytes", "tx_packets"}
	for i := 0; i < 4; i++ {
		bwValue := *(*uint64)(unsafe.Pointer(dataPtr + uintptr(offset)))
		bwMap[bwKeys[i]] = bwValue
		offset += 8
	}
	assessorData.BW[nicName] = bwMap
	//tcpconn
	tcpConnMap := make(map[string]uint64)
	tcpConnKeys := []string{"TCP_SYN_RECV", "TCP_ESTABLISHED", "TCP_TIME_WAIT"}
	for i := 0; i < 3; i++ {
		tcpConnValue := *(*uint64)(unsafe.Pointer(dataPtr + uintptr(offset)))
		tcpConnMap[tcpConnKeys[i]] = tcpConnValue
		offset += 8
	}
	assessorData.TCPConn = tcpConnMap
	//snmp
	snmpMap := make(map[string]uint64)
	var snmpKeys []string = []string{
		"TcpExtIPReversePathFilter",

		"IcmpInMsgs",

		"TcpOutRsts",
		"TcpInSegs",
		"TcpExtTCPReqQFullDrop",
		"TcpExtTCPReqQFullDoCookies",

		"UdpRcvbufErrors",
		"UdpNoPorts",
	}
	for i := 0; i < 8; i++ {
		snmpValue := *(*uint64)(unsafe.Pointer(dataPtr + uintptr(offset)))
		snmpMap[snmpKeys[i]] = snmpValue
		offset += 8
	}
	assessorData.SNMP = snmpMap
	//get packet infos
	packets[nicName] = make([]gopacket.Packet, 0)
	for pID := 0; pID < int(countTotal); pID++ {
		pLength := *(*uint16)(unsafe.Pointer(dataPtr + uintptr(offset)))
		offset += 2
		if pLength == 0 {
			break
		}
		pData := dataSlice[offset : offset+int(pLength)]
		offset += int(pLength)
		p := gopacket.NewPacket(pData, layers.LayerTypeEthernet, gopacket.NoCopy)
		packets[nicName] = append(packets[nicName], p)
	}
	return nicIP, assessorData, ratioMap, packets
}
