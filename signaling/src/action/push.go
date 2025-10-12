package action

import (
	"fmt"
	"net/http"
	"signaling/src/framework"
)

type pushAction struct{}

func NewPushAction() *pushAction {
	fmt.Println("push action")
	return &pushAction{}
}

func (*pushAction) Execute(w http.ResponseWriter, cr *framework.ComRequest) {

}
