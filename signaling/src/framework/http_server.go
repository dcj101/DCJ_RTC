package framework

import (
	"fmt"
	"net/http"
)

func init() {
	http.HandleFunc("/", entry)
}

type ActionInterface interface {
	Execute(w http.ResponseWriter, r *http.Request)
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
			action.Execute(w, r)
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
