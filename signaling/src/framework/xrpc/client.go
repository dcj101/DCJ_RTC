package xrpc

import (
	"bufio"
	"fmt"
	"net"
	"time"
)

const (
	DefaultConnectTimeout = 100 * time.Millisecond
	DefaultReadTimeout    = 500 * time.Millisecond
	DefaultWriteTimeout   = 500 * time.Millisecond
)

type Client struct {
	ConnectTimeout time.Duration
	ReadTimeout    time.Duration
	WriteTimeout   time.Duration
	Selector       ServerSelector
}

func NewClient(servers []string) *Client {
	// 有点类似c++的继承
	ss := new(RoundRobinSelector)
	if err := ss.AddServer(servers); err != nil {
		panic(err)
	}
	return &Client{
		Selector: ss,
	}
}

func (c *Client) connectTimeout() time.Duration {
	if c.ConnectTimeout == 0 {
		return DefaultConnectTimeout
	}
	return c.ConnectTimeout
}

func (c *Client) readTimeout() time.Duration {
	if c.ReadTimeout == 0 {
		return DefaultReadTimeout
	}
	return c.ReadTimeout
}

func (c *Client) writeTimeout() time.Duration {
	if c.WriteTimeout == 0 {
		return DefaultWriteTimeout
	}
	return c.WriteTimeout
}

func (c *Client) Do(req *Request) (*Response, error) {
	// 从选择器中选择一个服务器
	remoteAddr, err := c.Selector.PickServer()
	fmt.Println("remoteAddr:", remoteAddr)
	if err != nil {
		return nil, err
	}

	netConn, err := net.DialTimeout("tcp", remoteAddr.String(), c.connectTimeout())
	if err != nil {
		return nil, err
	}
	defer netConn.Close()

	netConn.SetReadDeadline(time.Now().Add(c.readTimeout()))
	netConn.SetWriteDeadline(time.Now().Add(c.writeTimeout()))

	// 发送请求
	// 使用 bufio.NewReadWriter 包装 netConn，提供带缓冲的读写能力
	// 1. NewReader: 预读取网络数据到内存，减少 Read 系统调用
	// 2. NewWriter: 暂存写入数据到内存，满后或 Flush 时才发送，减少 Write 系统调用
	rw := bufio.NewReadWriter(bufio.NewReader(netConn), bufio.NewWriter(netConn))
	if _, err := req.Write(rw); err != nil {
		// 写入请求头失败
		return nil, err
	}
	// 刷新缓冲区，确保请求被发送
	if err := rw.Flush(); err != nil {
		return nil, err
	}

	resp, err := ReadResponse(rw)
	if err != nil {
		return nil, err
	}

	return resp, nil
}
