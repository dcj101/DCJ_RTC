package xrpc

import (
	"errors"
	"net"
	"sync"
)

type ServerSelector interface {
	PickServer() (net.Addr, error)
}

type RoundRobinSelector struct {
	sync.RWMutex
	servers  []net.Addr
	curIndex int
}

func (rrs *RoundRobinSelector) AddServer(servers []string) error {
	if len(servers) == 0 {
		return errors.New("servers is empty")
	}

	addrs := make([]net.Addr, len(servers))
	for i, server := range servers {
		tcpAddr, err := net.ResolveTCPAddr("tcp", server)
		if err != nil {
			return err
		}
		addrs[i] = tcpAddr
	}

	rrs.Lock()
	rrs.servers = addrs
	rrs.Unlock()

	return nil
}

func (rrs *RoundRobinSelector) PickServer() (net.Addr, error) {
	rrs.Lock()
	rrs.curIndex = (rrs.curIndex + 1) % len(rrs.servers)
	rrs.Unlock()

	rrs.RLock()
	defer rrs.RUnlock()

	if len(rrs.servers) == 0 {
		return nil, errors.New("no server available")
	}

	return rrs.servers[rrs.curIndex], nil
}
