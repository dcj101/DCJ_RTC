#ifndef __XRTCSERVER_DEF_H_
#define __XRTCSERVER_DEF_H_

#define CMDNO_PUSH 1
#define CMDNO_PULL 2
#define CMDNO_ANSWER 3
#define CMDNO_STOPPUSH 4
#define CMDNO_STOPPULL 5

namespace xrtc {

struct RtcMsg {
    int cmdno;
    std::string stream_name;
    int audio;
    int video;
    uint64_t uid;
    void* certificate = nullptr;
    int log_id;
    // 到底是哪个signaling worker的conn，rtc需要回传信息
    void* woeker = nullptr;
    void* conn = nullptr;
    // 响应信息和错误码
    std::string sdp;
    int err_no = 0;
    std::string to_string() {
        return "cmdno: " + std::to_string(cmdno) +
            ", stream_name: " + stream_name +
            ", audio: " + std::to_string(audio) +
            ", video: " + std::to_string(video) +
            ", uid: " + std::to_string(uid) +
            ", log_id: " + std::to_string(log_id);
    }
};

}


#endif