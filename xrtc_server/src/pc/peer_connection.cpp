#include "peer_connection.h"

namespace xrtc {
PeerConnection::PeerConnection(EventLoop* el) : _el(el) {
}

PeerConnection::~PeerConnection() {
}

std::string PeerConnection::create_offer(const RTCOfferAnswerOptions& opts) {
    _local_desc = std::make_unique<SessionDescription>(SdpType::OFFER);
    if (opts.recv_audio) {
        _local_desc->add_media_desc(std::make_shared<AudioContentDescription>());
    }

    if (opts.recv_video) {
        _local_desc->add_media_desc(std::make_shared<VideoContentDescription>());
    }

    
    if (opts.use_rtp_mux) {
        ContentGroup group("BUNDLE");
        for (auto& desc : _local_desc->media_descs()) {
            group.add_content(desc->mid());
        }
        // 只有当bundle有内容时才添加group
        if (!group.contents().empty()) {
            _local_desc->add_content_group(group);
        }
    }
    

    return _local_desc->to_string();
}
}