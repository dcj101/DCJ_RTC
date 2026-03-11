#ifndef __PEER_CONNECTION_H_
#define __PEER_CONNECTION_H_
#include <string>
#include "base/event_loop.h"
#include "session_description.h"
namespace xrtc {

struct RTCOfferAnswerOptions {
    bool recv_audio = true;
    bool recv_video = true;
    bool use_rtp_mux = true;// 是否使用rtp mux bundle 功能 通道复用
};

class PeerConnection {
public:
    PeerConnection(EventLoop* el);
    ~PeerConnection();
    std::string create_offer(const RTCOfferAnswerOptions& opts);
private:
    EventLoop* _el;
    
    std::unique_ptr<SessionDescription> _local_desc;
    std::unique_ptr<SessionDescription> _remote_answer;
};

}

#endif