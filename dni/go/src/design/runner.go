package design

const (
	// 表示运行数据采集任务
	TELEMETRY_RUNNER string = "telemetry"
	// 表示运行数据清洗任务
	DATA_CLEANING_RUNNER string = "dataCleaning"
	// 表示运行数据统计任务
	DATA_STATISTICS_RUNNER string = "dataStatistics"
	// 表示运行数值相加任务
	DATA_SUM_RUNNER string = "Sum"
)

var Runners = []string{
	TELEMETRY_RUNNER,
	DATA_CLEANING_RUNNER,
	DATA_STATISTICS_RUNNER,
	DATA_SUM_RUNNER,
}
