package framework

import (
	"github.com/golang/glog"
)

func Init() error {
	glog.SetLogDir("./log")
	glog.SetLogFileName("signaling")
	glog.SetLogToStderr(true)
	glog.SetLogLevel(0)
	return nil
}
