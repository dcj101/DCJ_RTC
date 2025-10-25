package xrpc

import (
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
	if err != nil {
		return nil, err
	}

	netConn, err := net.DialTimeout("tcp", remoteAddr.String(), c.connectTimeout())
	if err != nil {
		return nil, err
	}

	netConn.SetReadDeadline(time.Now().Add(c.readTimeout()))
	netConn.SetWriteDeadline(time.Now().Add(c.writeTimeout()))

	return nil, nil
}
