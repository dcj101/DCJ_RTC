package framework

import "github.com/golang/glog"

func Init() error {
	glog.SetLogDir("./log")
	glog.SetLogFileName("signaling")
	return nil
}
