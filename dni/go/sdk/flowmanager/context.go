package flowmanager

type TaskContext struct {
	Inputs         *DataSlice
	Outputs        *DataSlice
	InputSideData  *DataSlice
	OutputSideData *DataSlice
}
