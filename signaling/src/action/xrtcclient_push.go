package action

import "net/http"

type XrtcClientPushAction struct {
}

func NewXrtcClientPushAction() *XrtcClientPushAction {
	return &XrtcClientPushAction{}
}

/*
(a *XrtcClientPushAction)：表示这是XrtcClientPushAction类型指针的方法，a是接收者的参数名（类似C++的this指针）
Execute：方法名
w http.ResponseWriter 和 r *http.Request：方法参数，用于HTTP响应和请求处理
*/

func (a *XrtcClientPushAction) Execute(w http.ResponseWriter, r *http.Request) {
	w.WriteHeader(http.StatusOK)
	w.Write([]byte("xrtc client push"))
}
