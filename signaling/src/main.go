package main

import (
	"fmt"
	"signaling/src/framework"
)

func main() {
	port := ":8080"
	if err := framework.StartHttp(port); err != nil {
		fmt.Println("http server start failed, err:", err)
	}
}
