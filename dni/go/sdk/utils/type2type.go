package utils

import "strconv"

func StringArrayToFloatArray(strs []string) ([]float32, error) {
	num := len(strs)
	floatArray := make([]float32, num)
	for i := 0; i < num; i++ {
		floatValue, err := strconv.ParseFloat(strs[i], 32)
		if err != nil {
			return floatArray, err
		}
		floatArray[i] = float32(floatValue)
	}
	return floatArray, nil
}
