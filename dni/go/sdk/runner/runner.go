package runner

import (
	"log"

	flowmng "github.com/amianetworks/dni/sdk/flowmanager"
)

const (
	PACKET_SAMPLING_RUNNER        string = "PacketSamplingRunner"
	PACKET_COUNT_FROM_PCAP_RUNNER string = "PacketCountFromPCAPRunner"
	INT_SUM_RUNNER                string = "IntSumRunner"
	MULTI_IN_OUT_TEST_RUNNER      string = "MultiInOutTestRunner"
	ONNX_RUNNER                   string = "OnnxRunner"
)

type Executor interface {
	Start([]*flowmng.DataSpec) ([]flowmng.DataSpec, error)
	Stop() error
}

var ExecutorRegistry = make(map[string]func(string, map[string]interface{}) Executor)

func RegisterExecutor(runner string, producer func(string, map[string]interface{}) Executor) {
	ExecutorRegistry[runner] = producer
}

// register all internal excutor
func InitExecutorRegistry() {
	RegisterExecutor("SumRunner", NewSumExecutor)
	RegisterExecutor("MultiInOutTestRunner", NewMultiOutTestExecutor)
	RegisterExecutor("OnnxRunner", NewOnnxExecutor)
	RegisterExecutor("ThresholdRunner", NewThresholdExecutor)
	RegisterExecutor("ConditionThresholdRunner", NewConditionThresholdExecutor)
	RegisterExecutor("CountRunner", NewCountExecutor)
	RegisterExecutor("MaxRunner", NewMaxExecutor)
	RegisterExecutor("AbnormalJudgeRunner", NewAbnormalJudgeExecutor)
	RegisterExecutor("PcapParseExecutor", NewPcapParseExecutor)
	RegisterExecutor("PacketFeatureRunner", NewPacketFeatureExecutor)
	RegisterExecutor("NumberStatisticRunner", NewNumberStatisticExecutor)
	RegisterExecutor("ProtoStatisticRunner", NewProtoStatisticExecutor)
	RegisterExecutor("AttackIPMergeRunner", NewAttackIPMergeExecutor)
	RegisterExecutor("SIPBaseMergeDedupRunner", NewSIPBaseMergeDedupExecutor)
	RegisterExecutor("SIPBaseMergeRunner", NewSIPBaseMergeDedupExecutor)
	RegisterExecutor("SndGenDMSRulesDedupRunner", NewSndGenDMSRulesDedupExecutor)
}

func NewRunnerExecutor(runner string, options map[string]interface{}) Executor {
	producer, ok := ExecutorRegistry[runner]
	if !ok {
		log.Printf("unsupported runner:%s", runner)
		return nil
	}
	e := producer(runner, options)
	return e
}
