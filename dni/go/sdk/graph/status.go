package scheduler

const (
	GRAPH_STATUS_NONE      string = "none"      // 图未完成初始化，空图
	GRAPH_STATUS_READY     string = "ready"     // 图任务已经ready，可以运行
	GRAPH_STATUS_RUNNING   string = "running"   // 图任务正在进行中
	GRAPH_STATUS_COMPLETED string = "completed" // 图任务已完成
	GRAPH_STATUS_FAILED    string = "failed"    // 图任务执行失败了
)

func (g *Graph) setStatusReady() {
	g.Status = GRAPH_STATUS_READY
}

func (g *Graph) setStatusRunning() {
	g.Status = GRAPH_STATUS_RUNNING
}

func (g *Graph) setStatusFailed() {
	g.Status = GRAPH_STATUS_FAILED
}
