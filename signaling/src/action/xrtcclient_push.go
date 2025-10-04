package action

import (
	"fmt"
	"html/template"
	"net/http"
)

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

func writeHtmlErrorResponse(w http.ResponseWriter, status int, err string) {
	w.WriteHeader(status)
	w.Write([]byte(fmt.Sprintf("%d - %s", status, err)))
}

func (a *XrtcClientPushAction) Execute(w http.ResponseWriter, r *http.Request) {
	t, err := template.ParseFiles("./static/template/push.tpl")
	if err != nil {
		fmt.Println("parse template failed, err:", err)
		writeHtmlErrorResponse(w, http.StatusNotFound, err.Error())
		return
	}

	request := make(map[string]string)

	// 关键：添加表单解析步骤
	err = r.ParseForm()
	if err != nil {
		fmt.Println("parse form failed, err:", err)
	}

	// 现在可以正确获取表单数据了
	for key, values := range r.Form {
		request[key] = values[0]
		fmt.Println("key:", key, "value:", values[0])
	}

	// 渲染模板
	err = t.Execute(w, request)
	if err != nil {
		fmt.Println("execute template failed, err:", err)
		writeHtmlErrorResponse(w, http.StatusNotFound, err.Error())
		return
	}

	fmt.Println("execute template success")
}
