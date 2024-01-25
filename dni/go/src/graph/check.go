package graph

import (
	"fmt"

	"github.com/amianetworks/dni/src/design"
)

func runnerIsValid(runner string) error {
	for _, rn := range design.Runners {
		if rn == runner {
			return nil
		}
	}
	return fmt.Errorf("runner <%s> does not exist", runner)
}

func inputFromIsValid(graphInput, nodeInput string, i int) error {
	switch i {
	case 1:
		if nodeInput != graphInput {
			return fmt.Errorf("first node input stream must be graph input stream")
		}
	default:
		if nodeInput != graphInput && nodeInput != design.INPUT_FROM_NODES {
			return fmt.Errorf("node input stream must from graph input or nodes")
		}
	}
	return nil
}

func outputToIsValid(graphOutput, nodeOutput string, nodeNum, i int) error {
	switch i {
	case nodeNum:
		if graphOutput != nodeOutput {
			return fmt.Errorf("last node output stream must be graph out put stream")
		}
	default:
		if nodeOutput != graphOutput && nodeOutput != design.OUTPUT_TO_NODES {
			return fmt.Errorf("node output stream must to graph output or nodes")
		}
	}
	return nil
}
