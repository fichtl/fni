package task

import (
	"log"

	flowmng "github.com/amianetworks/dni/sdk/flowmanager"
)

type Task interface {
	Open(*flowmng.TaskContext) error
	Process(*flowmng.TaskContext) error
	Close() error
}

var TaskRegistry = make(map[string]func(string, interface{}) Task)

func RegisterTask(task string, producer func(string, interface{}) Task) {
	TaskRegistry[task] = producer
}

func NewTask(task string, options interface{}) Task {
	producer, ok := TaskRegistry[task]
	if !ok {
		log.Printf("unsupported task:%s", task)
		return nil
	}
	e := producer(task, options)
	return e
}
