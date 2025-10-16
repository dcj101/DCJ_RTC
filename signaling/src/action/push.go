package action

import (
	"fmt"
	"net/http"
	"signaling/src/commerrors"
	"signaling/src/framework"
	"strconv"
)

type pushAction struct{}

func NewPushAction() *pushAction {
	fmt.Println("push action")
	return &pushAction{}
}

func (*pushAction) Execute(w http.ResponseWriter, cr *framework.ComRequest) {
	r := cr.R

	// uid
	var strUid string
	if values, ok := r.Form["uid"]; ok {
		strUid = values[0]
	}

	uid, err := strconv.ParseUint(strUid, 10, 61)

	if err != nil || uid <= 0 {
		cerr := commerrors.New(commerrors.ParamErr, "parse uid error:"+err.Error())
		writeJsonErrorResponse(cerr, w, cr)
	}

	// streamName
	var streamName string
	if values, ok := r.Form["streamName"]; ok {
		streamName = values[0]
	}

	if "" == streamName {
		cerr := commerrors.New(commerrors.ParamErr, "streamName is null")
		writeJsonErrorResponse(cerr, w, cr)
		return
	}

	// audio video
	var strAudio, strVideo string
	var audio, video int

	if values, ok := r.Form["audio"]; ok {
		strAudio = values[0]
	}

	if "" == strAudio || "0" == strAudio {
		audio = 0
	} else {
		audio = 1
	}

	if values, ok := r.Form["video"]; ok {
		strVideo = values[0]
	}

	if "" == strVideo || "0" == strVideo {
		video = 0
	} else {
		video = 1
	}

	fmt.Println(audio, video, strAudio, strVideo)

}
