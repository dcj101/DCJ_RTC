package main

import (
	"fmt"
	"net/http"
	"os"
	"os/signal"
	"syscall"
)

func startHttp(port string) error {
	fmt.Println("http server start on port:", port)
	err := http.ListenAndServe(port, nil)
	if err != nil {
		fmt.Println("http server start failed, err:", err)
		return err
	}

	return nil
}

func startHttps(port string, certFile string, keyFile string) error {
	fmt.Println("https server start on port:", port)
	err := http.ListenAndServeTLS(port, certFile, keyFile, nil)
	if err != nil {
		fmt.Println("https server start failed, err:", err)
		return err
	}

	return nil
}

func main() {
	// 定义一个url前缀
	staticUrl := "/static/"
	// 定义一个fileserver
	fileServer := http.FileServer(http.Dir("./static"))
	// 注册fileserver
	http.Handle(staticUrl, http.StripPrefix(staticUrl, fileServer))
	// 启动http server
	port := ":8080"
	go startHttp(port)

	// 启动https server
	port = ":8443"
	certFile := "./config/server.crt"
	keyFile := "./config/server.key"
	// 无法阻塞main会直接失败
	go startHttps(port, certFile, keyFile)
	// 设置信号处理
	sigChan := make(chan os.Signal, 1)
	signal.Notify(sigChan, syscall.SIGINT, syscall.SIGTERM) // 监听中断和终止信号

	fmt.Println("Server running. Press Ctrl+C to stop.")
	<-sigChan // 阻塞等待信号

	fmt.Println("Server shutting down...")
	// 在这里可以添加清理代码
}
