package task

import (
	"fmt"
	"log"

	flowmng "github.com/amianetworks/dni/sdk/flowmanager"
	"github.com/mitchellh/mapstructure"
	ort "github.com/yalue/onnxruntime_go"
)

type OnnxModel struct {
	model       string
	session     *ort.DynamicAdvancedSession
	inputNames  []string
	outputNames []string
	inputDims   [][]int64
	dynamicDims []int64
	fixSizes    []int64
}

func NewOnnxModel(model string) *OnnxModel {
	om := &OnnxModel{
		model:       model,
		inputNames:  make([]string, 0),
		outputNames: make([]string, 0),
		inputDims:   make([][]int64, 0),
		dynamicDims: make([]int64, 0),
		fixSizes:    make([]int64, 0),
	}
	return om
}

func (om *OnnxModel) Load() error {
	//获取模型输入输出信息
	InputInfos, OutputInfos, err := ort.GetInputOutputInfo(om.model)
	if err != nil {
		return fmt.Errorf("get info error:%v", err)
	}
	inputNums := len(InputInfos)
	outputNums := len(OutputInfos)
	for i := 0; i < inputNums; i++ {
		om.inputDims = append(om.inputDims, InputInfos[i].Dimensions.Clone())
		om.inputNames = append(om.inputNames, InputInfos[i].Name)
		om.dynamicDims = append(om.dynamicDims, -1)
		om.fixSizes = append(om.fixSizes, 1)
		dynamicnum := 0
		for dim := 0; dim < len(om.inputDims[i]); dim++ {
			if om.inputDims[i][dim] == -1 {
				dynamicnum++
				om.dynamicDims[i] = int64(dim)
				if dynamicnum >= 2 {
					return fmt.Errorf("not support two or more dynamic dims")
				}
			} else {
				om.fixSizes[i] = om.fixSizes[i] * om.inputDims[i][dim]
			}
		}
	}
	for i := 0; i < outputNums; i++ {
		om.outputNames = append(om.outputNames, OutputInfos[i].Name)
	}
	//加载模型
	session, err := ort.NewDynamicAdvancedSession(om.model, om.inputNames, om.outputNames, nil)
	if err != nil {
		return fmt.Errorf("load model error:%v", err)
	}
	om.session = session
	return nil
}

func (om *OnnxModel) Inference(inputs [][]float32) ([]ort.ArbitraryTensor, error) {
	//输入节点个数确认
	if len(inputs) != len(om.inputDims) {
		return nil, fmt.Errorf("input nodes num error")
	}
	//创建输入张量（先计算动态维度，后创建）
	inputNum := len(om.inputDims)
	inputTensors := make([]ort.ArbitraryTensor, 0)
	for idx := 0; idx < inputNum; idx++ {
		dim := om.dynamicDims[idx]
		inputdims := make([]int64, len(om.inputDims[idx]))
		copy(inputdims, om.inputDims[idx])
		if dim != -1 {
			dimsize := int64(len(inputs[idx])) / om.fixSizes[idx]
			inputdims[dim] = dimsize
		}
		//转为张量
		inputTensor, err := ort.NewTensor(ort.Shape(inputdims), inputs[idx])
		if err != nil {
			return []ort.ArbitraryTensor{}, fmt.Errorf("create the input tensor%d error:%v", idx, err)
		}
		inputTensors = append(inputTensors, inputTensor)
	}
	//创建输出张量
	outputNums := len(om.outputNames)
	outputTensors := make([]ort.ArbitraryTensor, outputNums)
	//模型推理计算
	err := om.session.Run(inputTensors, outputTensors)
	if err != nil {
		return []ort.ArbitraryTensor{}, fmt.Errorf("execute the network error:%v", err)
	}
	return outputTensors, nil
}

func (om *OnnxModel) Destory() {
	om.session.Destroy()
}

type OnnxTask struct {
	TaskName   string
	onnxModel  *OnnxModel
	LastOutput []float32
}

type OnnxTaskOptions struct {
	ModelPath string `mapstructure:"model_path"`
}

func NewOnnxTask(task string, options interface{}) Task {
	//decode map[string]interface{} to onnx task options
	var opts OnnxTaskOptions
	err := mapstructure.Decode(options, &opts)
	if err != nil {
		log.Printf("%s options decode error:%v", task, err)
		return nil
	}
	//create onnx executor
	t := &OnnxTask{
		TaskName: task,
	}
	//load model
	model := opts.ModelPath
	t.onnxModel = NewOnnxModel(model)
	err = t.onnxModel.Load()
	if err != nil {
		log.Printf("load onnx model error: %v\n", err)
		return nil
	}
	return t
}

func (t *OnnxTask) Start(ctx *flowmng.TaskContext) error {
	inputdata, ok := ctx.Inputs.Values[0].Data.([][]float32)
	if !ok {
		return fmt.Errorf("[%s] cast error", t.TaskName)
	}
	outputTensor, err := t.onnxModel.Inference(inputdata)
	if err != nil {
		log.Printf("onnx executor inference error:%v", err)
		return fmt.Errorf("onnx executor inference error:%v", err)
	}
	ctx.Outputs.Values[0].Data = outputTensor
	return nil
}

func (t *OnnxTask) Stop() error {
	return nil
}

func init() {
	RegisterTask("OnnxTask", NewOnnxTask)
}
