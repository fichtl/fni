package utils

func MatchStringSlice(elem string, slice []string) bool {
	for _, value := range slice {
		if elem == value {
			return true
		}
	}
	return false
}

func MatchIntSlice(elem int, slice []int) bool {
	for _, value := range slice {
		if elem == value {
			return true
		}
	}
	return false
}
