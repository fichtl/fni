package utils

import (
	"fmt"
	"log"
	"syscall"
	"unsafe"

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
		// log.Println("head ptr:", sm.pointer.head)
		// log.Println("trail ptr:", sm.pointer.tail)
		// log.Printf("ring buffer is empty")
		return 0, false
	}
	dataptr := sm.dataShmAddr + uintptr(sm.pointer.head)
	sliceptr := (*v)(unsafe.Pointer(dataptr))
	sm.pointer.head = (sm.pointer.head + SlotSize) % uint32(sm.pointer.size)
	fmt.Printf("head ptr:%d\n", sm.pointer.head)
	fmt.Printf("read data:%v\n", (*sliceptr)[:100])
	return dataptr, true
}

func RingBufferDataParse(dataPtr uintptr) (string, uint64, []float64, []map[string]uint32) {
	//offset
	offset := 0
	dataSlice := (*(*v)(unsafe.Pointer(dataPtr)))[:]
	//hostnic name length
	nameLength := *(*uint16)(unsafe.Pointer(dataPtr + uintptr(offset)))
	offset += 2
	log.Printf("host nic name length:%d", nameLength)
	//get host nic name
	hostNicName := string(dataSlice[offset : offset+int(nameLength)])
	offset += int(nameLength)
	log.Printf("host nic name bytes:%v", dataSlice[2:2+nameLength])
	log.Printf("host nic name:%s", hostNicName)
	//get time stamp
	ts := *(*uint64)(unsafe.Pointer(dataPtr + uintptr(offset)))
	offset += 8
	log.Printf("timestamp:%d", ts)
	//get packet stats
	packetStats := make([]float64, 9)
	for statID := 0; statID < 9; statID++ {
		stat := *(*uint32)(unsafe.Pointer(dataPtr + uintptr(offset)))
		packetStats[statID] = float64(stat)
		offset += 4
		log.Printf("packet stat (%d):%f", statID, packetStats[statID])
	}
	//get packet infos
	pinfos := make([]map[string]uint32, 0)
	for pID := 0; pID < 10000; pID++ {
		pLength := *(*uint16)(unsafe.Pointer(dataPtr + uintptr(offset)))
		offset += 2
		log.Printf("packet (%d) length:%d", pID, pLength)
		if pLength == 0 {
			break
		}
		pData := dataSlice[offset : offset+int(pLength)]
		offset += int(pLength)
		p := gopacket.NewPacket(pData, layers.LayerTypeEthernet, gopacket.NoCopy)
		pinfo := GetPacketInfo(p)
		pinfos = append(pinfos, pinfo.ToMap())
		log.Printf("packet info:%v", pinfo)
	}
	return hostNicName, ts, packetStats, pinfos
}
