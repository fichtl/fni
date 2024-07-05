package utils

import "math"

func IsEqual(f1, f2 float64) bool {
	return math.Dim(math.Max(f1, f2), math.Min(f1, f2)) < 1e-6
}
