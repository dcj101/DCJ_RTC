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
};

}


#endif