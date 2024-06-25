package common

import (
	"bytes"
	"encoding/json"
	"fmt"
	"strings"
)

const (
	ACTION_DROP int8 = iota
	ACTION_LIMIT_PPS
	ACTION_LIMIT_BPS
	ACTION_BYPASS
)

type Rules interface {
	Add(...Rule)
	Delete(...Rule)
	Size() int
	Empty() bool
	Contains(Rule) bool
	Search(Rule) (*Rule, int)
	Update(...Rule) []Rule
	Values() map[Match]Action
	ToList() []Rule
	// Marshal() ([]byte, error)
	// Unmarshal([]byte) error
}

type Match struct {
	Type string `json:"Type" yaml:"Type"` // l3/l4/l3l4

	SrcIP string `json:"SrcIP,omitempty" yaml:"SrcIP,omitempty"`
	DstIP string `json:"DstIP,omitempty" yaml:"DstIP,omitempty"`
	// IPProto refers to proto field in IP header, includes ICMP, IPv4/v6, TCP, UDP, etc.
	IPProto string `json:"IPProtocol,omitempty" yaml:"IPProtocol,omitempty"`

	SrcPort string `json:"SrcPort,omitempty" yaml:"SrcPort,omitempty"`
	DstPort string `json:"DstPort,omitempty" yaml:"DstPort,omitempty"`
	// TProto refers to the protocol carried by L4 payloads. Though there is no field
	// that specifies the payload's protocol, by convention it's possible to
	// distinguished it by src port, dst port or payload data feature.
	TProto   string `json:"TransProtocol,omitempty" yaml:"TransProtocol,omitempty"`
	TCPFlags string `json:"TCPFlags,omitempty" yaml:"TCPFlags,omitempty"`
}

type Action struct {
	Drop int8   `json:"Action" yaml:"Action"`
	Rate uint64 `json:"LimitRate" yaml:"LimitRate"`
}

// Rule represents the smallest unit that communicated between ASN DMS
// controller/servicenode, and between DMS Actuator/Analyser/Actuator.
type Rule struct {
	Match
	Action
}

func (r *Rule) String() string {
	var buf strings.Builder
	fmt.Fprintf(&buf, "Type: %q", r.Type)
	switch r.Type {
	case "l3":
		fmt.Fprintf(&buf, ", Src: %q, Dst: %q", r.SrcIP, r.DstIP)
	case "l4":
		fmt.Fprintf(&buf, ", Proto: %q, SrcPort: %q, DstPort: %q", r.IPProto, r.SrcPort, r.DstPort)
	case "l3l4":
		fmt.Fprintf(&buf, ", Src: %q, Proto: %q, DstPort: %q", r.SrcIP, r.IPProto, r.DstPort)
	default:
		fmt.Fprintf(&buf, ", Match: %q", r.Match)
	}
	switch r.Drop {
	case ACTION_DROP:
		buf.WriteString(", Action: DROP")
	case ACTION_LIMIT_BPS:
		fmt.Fprintf(&buf, ", Action: ratelimit to %d bps", r.Rate)
	case ACTION_LIMIT_PPS:
		fmt.Fprintf(&buf, ", Action: ratelimit to %d pps", r.Rate)
	}
	return buf.String()
}

func NewRuleSet(rules ...Rule) *RuleSet {
	rs := &RuleSet{rules: make(map[Match]Action)}
	for _, r := range rules {
		rs.rules[r.Match] = r.Action
	}
	return rs
}

type RuleSet struct {
	rules map[Match]Action
}

func (rs *RuleSet) Search(rule Rule) (*Rule, int) {
	if action, ok := rs.rules[rule.Match]; ok {
		return &Rule{rule.Match, action}, 0
	}

	// no match hit, and no overlapped rules
	return nil, 0
}

func (rs *RuleSet) Add(rules ...Rule) {
	for _, r := range rules {
		rs.rules[r.Match] = r.Action
	}
}

func (rs *RuleSet) Delete(rules ...Rule) {
	for _, r := range rules {
		delete(rs.rules, r.Match)
	}
}

func (rs *RuleSet) Update(rules ...Rule) []Rule {
	var del = make([]Rule, len(rules))
	var i int

	for _, r := range rules {
		if a, ok := rs.rules[r.Match]; !ok {
			rs.rules[r.Match] = r.Action
		} else if a != r.Action {
			del[i] = Rule{r.Match, a}
			i++
			rs.rules[r.Match] = r.Action
		}
	}

	return del
}

func (rs *RuleSet) Size() int {
	return len(rs.rules)
}

func (rs *RuleSet) Empty() bool {
	return rs.Size() == 0
}

func (rs *RuleSet) Contains(rule Rule) bool {
	if action, ok := rs.rules[rule.Match]; !ok {
		return false
	} else {
		return action == rule.Action
	}
}

func (rs *RuleSet) Has(rule Rule) bool {
	_, ok := rs.rules[rule.Match]
	return ok
}

func (rs *RuleSet) Values() map[Match]Action {
	return rs.rules
}

func (rs *RuleSet) ToList() []Rule {
	var ruleList = make([]Rule, rs.Size())
	var idx = 0
	for m, a := range rs.rules {
		ruleList[idx] = Rule{m, a}
		idx++
	}
	return ruleList
}

func (rs *RuleSet) GenRule() chan Rule {
	c := make(chan Rule)
	go func() {
		for m, a := range rs.rules {
			c <- Rule{m, a}
		}
		close(c)
	}()
	return c
}

func (rs *RuleSet) ClearAll() {
	rs.rules = make(map[Match]Action)
}

// Warning: data racing
func (rs *RuleSet) DeleteRulesByNumber(num int) *RuleSet {
	ret := NewRuleSet()

	if num > rs.Size() {
		return ret
	}

	// var delRules = make([]Rule, num)
	var i = 0
	for r, a := range rs.rules {
		if i >= num {
			break
		}
		// delete(rs.rules, r)
		// delRules[i] = Rule{r, a}
		ret.rules[r] = a
		i++
	}
	return ret
}

func (rs *RuleSet) MarshalJSON() ([]byte, error) {
	n := len(rs.rules)
	if n == 0 {
		return []byte("null"), nil
	}
	var buf bytes.Buffer
	i := 0
	for r, a := range rs.rules {
		if i > 0 {
			buf.WriteByte(',')
		}
		blob, err := json.Marshal(Rule{r, a})
		if err != nil {
			return nil, err
		}
		buf.Write(blob)
		i++
	}
	buf.WriteByte(']')
	return buf.Bytes(), nil
}

func (rs *RuleSet) UnmarshalJSON(v []byte) error {
	var rules []Rule
	if err := json.Unmarshal(v, &rules); err != nil {
		return err
	}
	for _, r := range rules {
		rs.Add(r)
	}
	return nil
}

type RuleMap map[string][]Rule

// ParseRuleMap parse the `RuleMap` into a map of `Rules`
func ParseRuleMap(rules map[string][]Rule) (map[string]Rules, error) {
	var ret = make(map[string]Rules)
	for dev, rs := range rules {
		ret[dev] = NewRuleSet(rs...)
	}
	return ret, nil
}

func (m RuleMap) String() string {
	buf := new(strings.Builder)
	for dev, rules := range m {
		rs, _ := json.Marshal(rules)
		fmt.Fprintf(buf, "\t%s: %s\n", dev, rs)
	}
	return buf.String()
}
