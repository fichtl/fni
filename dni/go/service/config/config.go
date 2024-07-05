package config

import (
	"fmt"
	"path/filepath"

	"github.com/spf13/viper"
)

type DMSConfig struct {
	Shm   ShareMemoryConfig `mapstructure:"shm"`
	Graph []GraphConfig     `mapstructure:"graph"`
}

type ShareMemoryConfig struct {
	ShmKey     int `mapstructure:"key"`
	MemorySize int `mapstructure:"size"`
}

type GraphConfig struct {
	Name string `mapstructure:"name"`
	Path string `mapstructure:"path"`
}

func GetDMSConfig(file string) (*DMSConfig, error) {
	fileName := filepath.Base(file)
	filePath := filepath.Dir(file)

	v := viper.New()
	v.SetConfigName(fileName)
	v.SetConfigType("yaml")
	v.AddConfigPath(filePath)

	if err := v.ReadInConfig(); err != nil {
		return nil, fmt.Errorf("wrong format: %v", err)
	}

	var dmsConf DMSConfig
	if err := v.Unmarshal(&dmsConf); err != nil {
		return nil, err
	}
	return &dmsConf, nil
}
