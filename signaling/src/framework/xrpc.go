package framework

import (
	"bytes"
	"encoding/json"
	"errors"
	"fmt"
	"signaling/src/framework/xrpc"
	"strconv"
	"strings"
	"time"
)

var xrpcClients map[string]*xrpc.Client = make(map[string]*xrpc.Client)

func loadXrpc() error {
	sections := configFile.GetSectionList()
	fmt.Println("sections:", sections)
	for _, section := range sections {
		if !strings.HasPrefix(section, "xrpc.") {
			continue
		}

		mSection, err := configFile.GetSection(section)
		if err != nil {
			return err
		}

		// server
		values, ok := mSection["server"]
		if !ok {
			return errors.New("no server field in config file")
		}

		arrServer := strings.Split(values, ",")
		for i, server := range arrServer {
			arrServer[i] = strings.TrimSpace(server)
			fmt.Println("server:", arrServer[i])
		}

		client := xrpc.NewClient(arrServer)

		if values, ok := mSection["connectTimeout"]; ok {
			if connectTimeout, err := strconv.Atoi(values); err == nil {
				client.ConnectTimeout = time.Duration(connectTimeout) * time.Millisecond
			}
		}

		if values, ok := mSection["readTimeout"]; ok {
			if readTimeout, err := strconv.Atoi(values); err == nil {
				client.ReadTimeout = time.Duration(readTimeout) * time.Millisecond
			}
		}

		if values, ok := mSection["writeTimeout"]; ok {
			if writeTimeout, err := strconv.Atoi(values); err == nil {
				client.WriteTimeout = time.Duration(writeTimeout) * time.Millisecond
			}
		}

		xrpcClients[section] = client
	}

	return nil
}

func Call(serviceName string, req interface{}, resp interface{}, logId int32) error {
	fmt.Println("call service:", serviceName, "req:", req, "resp:", resp, "logId:", logId)
	client, ok := xrpcClients["xrpc."+serviceName]
	if !ok {
		return errors.New("no xrpc client found")
	}

	fmt.Println(client)

	content, err := json.Marshal(req)
	if err != nil {
		return err
	}

	cReq := xrpc.NewRequest(bytes.NewBuffer(content), uint32(logId))

	respBody, err := client.Do(cReq)
	if err != nil {
		return err
	}

	err = json.Unmarshal(respBody.Body, resp)

	return err
}
