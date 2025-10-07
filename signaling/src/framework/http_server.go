package framework

import (
	"fmt"
	"net/http"
)

func init() {
	http.HandleFunc("/", entry)
}

type ActionInterface interface {
	Execute(w http.ResponseWriter, r *ComRequest)
}

type ComRequest struct {
	R      *http.Request
	LogId  int32
	Logger *ComLog
}

func getClientRealIP(r *http.Request) string {
	ip := r.Header.Get("X-Real-IP")
	if ip == "" {
		ip = r.Header.Get("X-Forwarded-For")
	}
	if ip == "" {
		ip = r.RemoteAddr
	}
	return ip
}

var GActionRouter map[string]ActionInterface = make(map[string]ActionInterface)

func responseError(w http.ResponseWriter, r *http.Request, status int, err string) {
	w.WriteHeader(http.StatusInternalServerError)
	w.Write([]byte(fmt.Sprintf("%d - %s", status, err)))
}

func entry(w http.ResponseWriter, r *http.Request) {
	if r.URL.Path == "/favicon.ico" {
		w.WriteHeader(http.StatusOK)
		w.Write([]byte{})
		return
	}
	fmt.Println("request path:", r.URL.Path)
	if action, ok := GActionRouter[r.URL.Path]; ok {
		if action != nil {
			// 关键：添加表单解析步骤
			err := r.ParseForm()
			if err != nil {
				fmt.Println("parse form failed, err:", err)
			}

			cr := &ComRequest{
				R:      r,
				LogId:  GetLogIdInt32(),
				Logger: &ComLog{},
			}
			cr.Logger.AddLogItem("path", r.URL.Path)
			cr.Logger.AddLogItem("logid", fmt.Sprintf("%d", cr.LogId))
			cr.Logger.AddLogItem("referer", r.Referer())
			cr.Logger.AddLogItem("user_agent", r.UserAgent())
			cr.Logger.AddLogItem("remote_addr", r.RemoteAddr)
			cr.Logger.AddLogItem("ua", r.UserAgent())
			cr.Logger.AddLogItem("client_ip", getClientRealIP(r))

			for key, values := range r.Form {
				cr.Logger.AddLogItem(key, values[0])
			}
			cr.Logger.TimeBegin("execute")
			action.Execute(w, cr)
			cr.Logger.TimeEnd("execute")
			cr.Logger.Infof("")
		} else {
			responseError(w, r, http.StatusInternalServerError, "internal server error")
		}
	} else {
		responseError(w, r, http.StatusNotFound, "Not Found")
	}
}

func StartHttp(port string) error {
	fmt.Println("http server start on port:", port)
	return http.ListenAndServe(port, nil)
}
