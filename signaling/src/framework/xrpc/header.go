package xrpc

import (
	"encoding/binary"
	"errors"
	"io"
)

const (
	HEADER_SIZE     = 36
	HEADER_MAGICNUM = 0xfb202202
)

// Header 表示RPC请求头
// 包含请求ID、版本号、日志ID、提供方ID、魔术数、保留字段和请求体长度
type Header struct {
	Id       uint16
	Version  uint16
	LogId    uint32
	Provider [16]byte
	MagicNum uint32
	Reserved uint32
	BodyLen  uint32
}

// Marshal 将 Header 结构体序列化为 36 字节的二进制数据
// 使用 encoding/binary 包进行小端序 (LittleEndian) 编码
func (h *Header) Marshal(b []byte) error {
	if len(b) < HEADER_SIZE {
		return errors.New("no enough buffer for header")
	}
	// 把header的字段写入到b中
	binary.LittleEndian.PutUint16(b[0:2], h.Id)         // 0-1: 请求 ID (2 bytes)
	binary.LittleEndian.PutUint16(b[2:4], h.Version)    // 2-3: 协议版本 (2 bytes)
	binary.LittleEndian.PutUint32(b[4:8], h.LogId)      // 4-7: 日志追踪 ID (4 bytes)
	copy(b[8:24], h.Provider[:])                        // 8-23: 服务商标识 (16 bytes)
	binary.LittleEndian.PutUint32(b[24:28], h.MagicNum) // 24-27: 魔数 (4 bytes)
	binary.LittleEndian.PutUint32(b[28:32], h.Reserved) // 28-31: 保留字段 (4 bytes)
	binary.LittleEndian.PutUint32(b[32:36], h.BodyLen)  // 32-35: 包体长度 (4 bytes) -> 总计 36 字节

	return nil
}

func (h *Header) UnMarshal(b []byte) error {
	if len(b) < HEADER_SIZE {
		return errors.New("incomplete header")
	}

	h.Id = binary.LittleEndian.Uint16(b[0:2])
	h.Version = binary.LittleEndian.Uint16(b[2:4])
	h.LogId = binary.LittleEndian.Uint32(b[4:8])
	copy(h.Provider[:], b[8:24])
	h.MagicNum = binary.LittleEndian.Uint32(b[24:28])
	h.Reserved = binary.LittleEndian.Uint32(b[28:32])
	h.BodyLen = binary.LittleEndian.Uint32(b[32:36])

	return nil
}

func (h *Header) Write(w io.Writer) (n int, err error) {
	var buf [HEADER_SIZE]byte
	if err = h.Marshal(buf[:]); err != nil {
		return 0, err
	}
	// 写入header到io中
	return w.Write(buf[:])
}

func (h *Header) Read(r io.Reader) (n int, err error) {
	var buf [HEADER_SIZE]byte
	if n, err = io.ReadFull(r, buf[:]); err != nil {
		return 0, err
	}

	if err = h.UnMarshal(buf[:]); err != nil {
		return 0, err
	}

	return n, nil
}
