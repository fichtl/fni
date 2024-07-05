package analyzer

import "fmt"

type Query struct {
	Id   int
	Conn map[Measurement]struct{}
	Dp   map[string]Measurement
}

func NewQuery() *Query {
	return &Query{
		Conn: make(map[Measurement]struct{}),
		Dp:   make(map[string]Measurement),
	}
}

func (q *Query) String() string {
	return fmt.Sprintf("qid: %d, dp: %s, conn: %s", q.Id, q.Dp, q.Conn)
}

func (q *Query) SetDp(dev string, m uint8) {
	o := q.Dp[dev]
	q.Dp[dev] = Measurement{PROTO, o.L2 | m}
}

func (q *Query) IsNil() bool {
	return len(q.Dp) == 0 && len(q.Conn) == 0
}
