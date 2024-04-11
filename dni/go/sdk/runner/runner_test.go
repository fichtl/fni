package runner

import (
	"crypto/rand"
	"fmt"
	"math/big"
	"testing"

	flowmng "github.com/amianetworks/dni/sdk/flowmanager"
	"github.com/amianetworks/dni/sdk/utils"
	ort "github.com/yalue/onnxruntime_go"
)

// TODO:test onnx model
var onnxruntimeLibPath = "/home/yf/workspace/dni/dni/go/third_party/onnxruntime.so"

func TestLoadOnnxModel1(t *testing.T) {
	//Init environment
	ort.SetSharedLibraryPath(onnxruntimeLibPath)
	err := ort.InitializeEnvironment()
	if err != nil {
		t.Errorf("initialize the onnxruntime library error: %v", err)
		t.Fail()
	}
	model := "/home/yf/workspace/onnxruntime_go_examples/goonnx_test/models/model_2_2_x_2_2_v19.onnx"
	om := NewOnnxModel(model)
	if om == nil {
		t.Errorf("new onnx model error")
		t.Fail()
	}
	err = om.Load()
	if err != nil {
		t.Errorf("get info error:%v", err)
		t.Fail()
	}
	fmt.Println(om.inputNames)
	fmt.Println(om.inputDims)
	fmt.Println(om.outputNames)
}

func TestOnnxModelInference1(t *testing.T) {
	//Init environment
	ort.SetSharedLibraryPath(onnxruntimeLibPath)
	err := ort.InitializeEnvironment()
	if err != nil {
		t.Errorf("initialize the onnxruntime library error: %v", err)
		t.Fail()
	}
	model := "/home/yf/workspace/onnxruntime_go_examples/goonnx_test/models/model_2_2_x_2_2_v19.onnx"
	//new model
	om := NewOnnxModel(model)
	if om == nil {
		t.Errorf("new onnx model error")
		t.Fail()
	}
	//get info
	err = om.Load()
	if err != nil {
		t.Errorf("get info error:%v", err)
		t.Fail()
	}
	fmt.Println(om.inputDims)
	//input
	inputs := [][]float32{{1, 2, 3, 4}, {3, 3, 3, 3}, {2, 2, 2, 2}}
	outputTesnsor, err := om.Inference(inputs)
	if err != nil {
		t.Errorf("inference error:%v", err)
		t.Fail()
	}
	//get output
	for i := 0; i < len(outputTesnsor); i++ {
		outdata := outputTesnsor[i].(*ort.Tensor[float32]).GetData()
		fmt.Printf("output%d:%v", i, outdata)
	}
}

func TestLoadOnnxModel2(t *testing.T) {
	//Init environment
	ort.SetSharedLibraryPath(onnxruntimeLibPath)
	err := ort.InitializeEnvironment()
	if err != nil {
		t.Errorf("initialize the onnxruntime library error: %v", err)
		t.Fail()
	}
	model := "/home/yf/workspace/onnxruntime_go_examples/goonnx_test/models/dynamic_model.onnx"
	om := NewOnnxModel(model)
	if om == nil {
		t.Errorf("new onnx model error")
		t.Fail()
	}
	err = om.Load()
	if err != nil {
		t.Errorf("get info error:%v", err)
		t.Fail()
	}
	fmt.Println(om.inputNames)
	fmt.Println(om.inputDims)
	fmt.Println(om.outputNames)
}

func TestOnnxModelInference2(t *testing.T) {
	//Init environment
	ort.SetSharedLibraryPath(onnxruntimeLibPath)
	err := ort.InitializeEnvironment()
	if err != nil {
		t.Errorf("initialize the onnxruntime library error: %v", err)
		t.Fail()
	}
	model := "/home/yf/workspace/onnxruntime_go_examples/goonnx_test/models/dynamic_model.onnx"
	//new model
	om := NewOnnxModel(model)
	if om == nil {
		t.Errorf("new onnx model error")
		t.Fail()
	}
	//get info
	err = om.Load()
	if err != nil {
		t.Errorf("get info error:%v", err)
		t.Fail()
	}
	fmt.Println(om.inputDims)
	//input
	inputs := [][]float32{{1, 2, 3, 4}, {3, 3, 2, 2}}
	outputTesnsor, err := om.Inference(inputs)
	if err != nil {
		t.Errorf("inference error:%v", err)
		t.Fail()
	}
	//get output
	for i := 0; i < len(outputTesnsor); i++ {
		outdata := outputTesnsor[i].(*ort.Tensor[float32]).GetData()
		fmt.Printf("output%d:%v", i, outdata)
	}
}

func TestNewOnnxExecutor(t *testing.T) {
	//Init environment
	ort.SetSharedLibraryPath(onnxruntimeLibPath)
	err := ort.InitializeEnvironment()
	if err != nil {
		t.Errorf("initialize the onnxruntime library error: %v", err)
		t.Fail()
	}
	//register
	InitExecutorRegistry()
	//cretae executor
	options := map[string]interface{}{"path": "/home/yf/workspace/onnxruntime_go_examples/goonnx_test/models/model_2_2_x_2_2_v19.onnx"}
	e := NewRunnerExecutor("OnnxRunner", options)
	if e == nil {
		t.Errorf("cretae onnx executor error")
	}
}

func TestStartOnnxExecutor(t *testing.T) {
	//Init environment
	ort.SetSharedLibraryPath(onnxruntimeLibPath)
	err := ort.InitializeEnvironment()
	if err != nil {
		t.Errorf("initialize the onnxruntime library error: %v", err)
		t.Fail()
	}
	//register
	InitExecutorRegistry()
	//init onnx executor
	options := map[string]interface{}{"path": "/home/yf/workspace/onnxruntime_go_examples/goonnx_test/models/model_2_2_x_2_2_v19.onnx"}
	e := NewRunnerExecutor("OnnxRunner", options)
	if e == nil {
		t.Errorf("cretae onnx executor error")
		t.Fail()
		return
	}
	//start onnx executor
	inputs := [][]float32{{1, 2, 3, 4}, {3, 3, 3, 3}, {2, 2, 2, 2}}
	data := flowmng.DataSpec{
		Data: inputs,
	}
	datas := []*flowmng.DataSpec{&data}
	outputData, err := e.Start(datas)
	if err != nil {
		t.Errorf("start onnx executor error")
		t.Fail()
	}
	//print res
	outputTensors := outputData[0].Data.([]ort.ArbitraryTensor)
	fmt.Println(outputTensors)
	for i := 0; i < len(outputTensors); i++ {
		output := outputTensors[i].(*ort.Tensor[float32]).GetData()
		fmt.Printf("output %d:%v", i, output)
	}
}

func TestNewThresholdExecutor(t *testing.T) {
	//Init environment
	//register
	InitExecutorRegistry()
	//cretae executor
	options := map[string]interface{}{
		"thresholds": []float64{60, 100, 1000},
		"scores":     []float64{0, 1, 2, 3},
	}
	e := NewRunnerExecutor("ThresholdRunner", options)
	if e == nil {
		t.Errorf("cretae threshold executor error")
	}
	//input data
	d := flowmng.DataSpec{
		Data: float64(10000),
	}
	outputs, err := e.Start([]*flowmng.DataSpec{&d})
	if err != nil {
		t.Errorf("threshold executor start error:%v", err)
	}
	t.Log(outputs)
}

func TestNewCondThresholdExecutor(t *testing.T) {
	//Init environment
	//register
	InitExecutorRegistry()
	//cretae executor
	options := map[string]interface{}{
		"condthreshold": float64(200),
		"ncondscore":    float64(3),
		"thresholds":    []float64{60, 100},
		"scores":        []float64{0, 1, 2},
	}
	e := NewRunnerExecutor("ConditionThresholdRunner", options)
	if e == nil {
		t.Errorf("cretae threshold executor error")
	}
	//input data
	d1 := flowmng.DataSpec{
		Data: float64(70),
	}
	d2 := flowmng.DataSpec{
		Data: float64(300),
	}
	outputs, err := e.Start([]*flowmng.DataSpec{&d1, &d2})
	if err != nil {
		t.Errorf("threshold executor start error:%v", err)
	}
	t.Log(outputs)
}

func TestNewSumExecutor(t *testing.T) {
	//Init environment
	//register
	InitExecutorRegistry()
	//cretae executor
	e := NewRunnerExecutor("SumRunner", nil)
	if e == nil {
		t.Errorf("cretae sum executor error")
	}
	//input data
	d1 := flowmng.DataSpec{
		Data: float64(70),
	}
	d2 := flowmng.DataSpec{
		Data: float64(200),
	}
	d3 := flowmng.DataSpec{
		Data: float64(500),
	}
	outputs, err := e.Start([]*flowmng.DataSpec{&d1, &d2, &d3})
	if err != nil {
		t.Errorf("sum executor start error:%v", err)
	}
	t.Log(outputs)
}

func TestNewCountExecutor(t *testing.T) {
	//Init environment
	//register
	InitExecutorRegistry()
	//options
	options := map[string]interface{}{
		"specifiedvalue": float64(200),
	}
	//cretae executor
	e := NewRunnerExecutor("CountRunner", options)
	if e == nil {
		t.Errorf("cretae sum executor error")
	}
	//input data
	d1 := flowmng.DataSpec{
		Data: float64(70),
	}
	d2 := flowmng.DataSpec{
		Data: float64(200),
	}
	d3 := flowmng.DataSpec{
		Data: float64(200),
	}
	outputs, err := e.Start([]*flowmng.DataSpec{&d1, &d2, &d3})
	if err != nil {
		t.Errorf("sum executor start error:%v", err)
	}
	t.Log(outputs)
}

func TestMaxExecutor(t *testing.T) {
	//Init environment
	//register
	InitExecutorRegistry()
	//cretae executor
	e := NewRunnerExecutor("MaxRunner", nil)
	if e == nil {
		t.Errorf("cretae sum executor error")
	}
	//input data
	d1 := flowmng.DataSpec{
		Data: float64(70),
	}
	d2 := flowmng.DataSpec{
		Data: float64(200),
	}
	d3 := flowmng.DataSpec{
		Data: float64(400),
	}
	outputs, err := e.Start([]*flowmng.DataSpec{&d1, &d2, &d3})
	if err != nil {
		t.Errorf("sum executor start error:%v", err)
	}
	t.Log(outputs)
}

func TestPacketFeatureExecutor(t *testing.T) {
	//Init environment
	//register
	InitExecutorRegistry()
	//options
	options := map[string]interface{}{
		"featurenames": []string{"SPort", "DPort"},
	}
	//cretae executor
	e := NewRunnerExecutor("PacketFeatureRunner", options)
	if e == nil {
		t.Errorf("cretae sum executor error")
	}
	//input data
	d := flowmng.DataSpec{}
	d.Data = []map[string]uint32{
		{"SPort": 1, "DPort": 2, "Proto": 0},
		{"SPort": 2, "DPort": 2, "Proto": 0},
		{"SPort": 3, "DPort": 4, "Proto": 0},
		{"SPort": 1, "DPort": 5, "Proto": 0},
		{"SPort": 1, "DPort": 2, "Proto": 0},
		{"SPort": 2, "DPort": 6, "Proto": 0},
	}
	outputs, err := e.Start([]*flowmng.DataSpec{&d})
	if err != nil {
		t.Errorf("sum executor start error:%v", err)
	}
	t.Log(outputs)
}

func TestAttackIPMergeExecutor1(t *testing.T) {
	//Init environment
	//register
	InitExecutorRegistry()
	//options
	options := map[string]interface{}{
		"ipCountSum":           10000,
		"ipFw4CountRatio":      0.1,
		"ipFw3CountRatio":      0.2,
		"ipFw2CountRatio":      0.4,
		"ipRandCountRatio":     0.5,
		"ipSegCoverThreshold":  100,
		"ipRandCountThreshold": 2,
	}
	//cretae executor
	e := NewRunnerExecutor("AttackIPMergeRunner", options)
	if e == nil {
		t.Errorf("cretae sum executor error")
	}
	//input data
	ip1 := "1.2.3.0"
	ip2 := "1.2.3.5"
	ipCountDF := map[uint32]int{
		utils.Atoui(ip1): 5000,
		utils.Atoui(ip2): 5000,
	}
	d1 := flowmng.DataSpec{
		Data: ipCountDF}
	d2 := flowmng.DataSpec{
		Data: make(map[uint32]struct{}),
	}
	outputs, err := e.Start([]*flowmng.DataSpec{&d1, &d2})
	if err != nil {
		t.Errorf("sum executor start error:%v", err)
	}
	t.Log(outputs)
}

func TestAttackIPMergeExecutor2(t *testing.T) {
	//Init environment
	//register
	InitExecutorRegistry()
	//options
	options := map[string]interface{}{
		"ipCountSum":           10000,
		"ipFw4CountRatio":      0.1,
		"ipFw3CountRatio":      0.2,
		"ipFw2CountRatio":      0.4,
		"ipRandCountRatio":     0.5,
		"ipSegCoverThreshold":  100,
		"ipRandCountThreshold": 2,
	}
	//cretae executor
	e := NewRunnerExecutor("AttackIPMergeRunner", options)
	if e == nil {
		t.Errorf("cretae sum executor error")
	}
	//input data
	ips := make([]uint32, 0)
	ip := utils.Atoui("1.2.3.0")
	for i := 0; i < 10000; i++ {
		randnum, _ := rand.Int(rand.Reader, big.NewInt(256))
		ips = append(ips, ip+uint32(randnum.Int64()))
	}
	ipCountDF := make(map[uint32]int)
	for _, ip := range ips {
		ipCountDF[ip]++
	}
	d1 := flowmng.DataSpec{
		Data: ipCountDF}
	d2 := flowmng.DataSpec{
		Data: make(map[uint32]struct{}),
	}
	outputs, err := e.Start([]*flowmng.DataSpec{&d1, &d2})
	if err != nil {
		t.Errorf("sum executor start error:%v", err)
	}
	t.Log(outputs)
}

func TestAttackIPMergeExecutor3(t *testing.T) {
	//Init environment
	//register
	InitExecutorRegistry()
	//options
	options := map[string]interface{}{
		"ipCountSum":           10000,
		"ipFw4CountRatio":      0.1,
		"ipFw3CountRatio":      0.2,
		"ipFw2CountRatio":      0.4,
		"ipRandCountRatio":     0.5,
		"ipSegCoverThreshold":  100,
		"ipRandCountThreshold": 2,
	}
	//cretae executor
	e := NewRunnerExecutor("AttackIPMergeRunner", options)
	if e == nil {
		t.Errorf("cretae sum executor error")
	}
	//input data
	ips := make([]uint32, 0)
	ip := utils.Atoui("1.2.0.0")
	for i := 0; i < 10000; i++ {
		randnum, _ := rand.Int(rand.Reader, big.NewInt(256))
		ips = append(ips, ip+256*uint32(randnum.Int64()))
	}
	ipCountDF := make(map[uint32]int)
	for _, ip := range ips {
		ipCountDF[ip]++
	}
	d1 := flowmng.DataSpec{
		Data: ipCountDF}
	d2 := flowmng.DataSpec{
		Data: make(map[uint32]struct{}),
	}
	outputs, err := e.Start([]*flowmng.DataSpec{&d1, &d2})
	if err != nil {
		t.Errorf("sum executor start error:%v", err)
	}
	t.Log(outputs)
}
