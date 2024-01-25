package node

import (
	"github.com/amianetworks/am.modules/db/src/tsdb"
	"github.com/amianetworks/dni/src/data"
	"github.com/amianetworks/dni/src/design"
)

type DataCleanInputHandler struct {
	From     string
	DBConf   design.DataBaseConfig
	DBClient tsdb.Database
	DBColl   tsdb.Collection

	OnOff chan struct{}
}

type DataCleanOutputHandler struct {
	To   string
	File string
}

func (dcih *DataCleanInputHandler) PrepareForRun() error {

	return nil
}

func (dcih *DataCleanInputHandler) ReceiveInputStreams(inputChan *data.InputManager) error {
	// query := tsdb.NewQuery()
	// ticker := time.NewTicker(1 * time.Second)

	// var latestTimestamp int64

	// go func() {
	// 	for {
	// 		select {
	// 		case <-ticker.C:
	// 			dp, err := dcih.DBColl.FindDataPoints(query)
	// 			if err != nil {
	// 				log.R.Errorf("failed to find data from DB")
	// 			}
	// 			for fieldName, mtrix := range dp.Results {
	// 				for tags, samples := range mtrix {
	// 					for _, v := range samples.Values {
	// 						if latestTimestamp < int64(v.Timestamp) {
	// 							latestTimestamp = int64(v.Timestamp)
	// 							break
	// 						}
	// 					}
	// 				}
	// 			}
	// 		case <-dcih.dbOnoff:
	// 			return
	// 		}
	// 	}
	// }()
	return nil
}

func (dcoh *DataCleanOutputHandler) PrepareForRun() error {
	return nil
}
