package framework

import (
	"os"

	"github.com/golang/glog"
)

type LogLevel int32

const (
	LogLevelDebug LogLevel = iota
	LogLevelInfo
	LogLevelWarning
	LogLevelError
	LogLevelFatal
)

func getLogLevel(level string) LogLevel {
	switch level {
	case "debug":
		return LogLevelDebug
	case "info":
		return LogLevelInfo
	case "warning":
		return LogLevelWarning
	case "error":
		return LogLevelError
	case "fatal":
		return LogLevelFatal
	default:
		return LogLevelInfo
	}
}

var gconf *FrameworkConf

func Init(configDir string) error {
	var err error
	gconf, err = loadConfig(configDir)
	if err != nil {
		glog.Error("load config failed, err: %v", err)
		return err
	}
	// 检查日志目录是否存在
	logDir := gconf.GetLogDir()
	if _, err := os.Stat(logDir); err == nil {
		// 目录存在，删除它及其中的所有内容
		if err := os.RemoveAll(logDir); err != nil {
			return err
		}
	}

	// 重新创建日志目录
	if err := os.MkdirAll(logDir, 0755); err != nil {
		return err
	}
	glog.SetLogDir(logDir)
	glog.SetLogFileName(gconf.GetLogFile())
	glog.SetAlsoToStderr(gconf.GetLogToStderr())
	glog.SetLogLevel(int32(getLogLevel(gconf.GetLogLevel())))
	return nil
}
