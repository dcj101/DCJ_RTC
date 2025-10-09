package main

import (
	"flag"
	"fmt"
	"signaling/src/framework"

	"github.com/golang/glog"
)

func main() {
	flag.Parse()
	if err := framework.Init("./conf/framework.conf"); err != nil {
		fmt.Println("init failed, err:", err)
	}

	glog.Info("signaling server start")
	glog.Debug("signaling server debug")
	port := ":8080"
	err := framework.StartHttp(port)
	if err != nil {
		fmt.Println("http server start failed, err:", err)
		panic(err)
	}
}
