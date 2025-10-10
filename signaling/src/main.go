package main

import (
	"flag"
	"fmt"
	"os"
	"os/signal"
	"signaling/src/framework"
	"syscall"

	"github.com/golang/glog"
)

func main() {
	flag.Parse()
	if err := framework.Init("./conf/framework.conf"); err != nil {
		fmt.Println("init failed, err:", err)
	}

	framework.RegisterStaticFileServer()
	go StartHttpServer()
	go StartHttpsServer()

	glog.Info("signaling server start")
	// 设置信号处理
	sigChan := make(chan os.Signal, 1)
	signal.Notify(sigChan, syscall.SIGINT, syscall.SIGTERM) // 监听中断和终止信号

	fmt.Println("Server running. Press Ctrl+C to stop.")
	<-sigChan // 阻塞等待信号

	fmt.Println("Server shutting down...")
	// 在这里可以添加清理代码
}

func StartHttpServer() {
	err := framework.StartHttp()
	if err != nil {
		fmt.Println("http server start failed, err:", err)
		panic(err)
	}
}

func StartHttpsServer() {
	err := framework.StartHttps()
	if err != nil {
		fmt.Println("https server start failed, err:", err)
		panic(err)
	}
}
