package main

import (
	"fmt"
	"net/http"
)

func main() {
	// 定义一个url前缀
	staticUrl := "/static/"
	// 定义一个fileserver
	fileServer := http.FileServer(http.Dir("./static"))
	// 注册fileserver
	http.Handle(staticUrl, http.StripPrefix(staticUrl, fileServer))
	// 启动http server
	err := http.ListenAndServe(":8080", nil)
	if err != nil {
		fmt.Println("http server start failed, err:", err)
		return
	}
}
