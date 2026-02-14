package xrpc

import (
	"bytes"
	"io"
)

// Request 表示一个RPC请求
// 包含请求头和请求体
type Request struct {
	Header Header
	Body   io.Reader
}

func NewRequest(body io.Reader, logId uint32) *Request {
	req := new(Request)
	req.Header.LogId = logId
	req.Header.MagicNum = HEADER_MAGICNUM

	if body != nil {
		switch body.(type) {
		case *bytes.Buffer:
			req.Header.BodyLen = uint32(body.(*bytes.Buffer).Len())
		default:
			return nil
		}

		req.Body = io.LimitReader(body, int64(req.Header.BodyLen))
	}

	return req
}

func (r *Request) Write(w io.Writer) (n int, err error) {
	// 固定header大小
	if n, err = r.Header.Write(w); err != nil {
		return 0, err
	}
	// 写入body
	written, err := io.Copy(w, r.Body)
	return int(written), err
}
