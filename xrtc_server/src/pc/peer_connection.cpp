#include "peer_connection.h"
#include "ice/ice_credentials.h"

namespace xrtc {

static RtpDirection get_rtp_dir(bool send, bool recv) {
    if (send && recv) {
        return RtpDirection::SendReceive;
    } else if (send) {
        return RtpDirection::SendOnly;
    } else if (recv) {
        return RtpDirection::ReceiveOnly;
    } else {
        return RtpDirection::Inactive;
    }
}

PeerConnection::PeerConnection(EventLoop* el) : _el(el) {
}

PeerConnection::~PeerConnection() {
}

std::string PeerConnection::create_offer(const RTCOfferAnswerOptions& opts) {
    _local_desc = std::make_unique<SessionDescription>(SdpType::OFFER);
    IceParameters ice_params = IceCredentials::create_random_ice_credentials();

    if (opts.recv_audio) {
        auto audio_desc = std::make_shared<AudioContentDescription>();
        audio_desc->set_direction(get_rtp_dir(opts.send_audio, opts.recv_audio));
        audio_desc->set_rtcp_mux(opts.use_rtcp_mux);
        _local_desc->add_media_desc(audio_desc);
        _local_desc->add_transport_info(audio_desc->mid(), ice_params);
    }

    if (opts.recv_video) {
        auto video_desc = std::make_shared<VideoContentDescription>();
        video_desc->set_direction(get_rtp_dir(opts.send_video, opts.recv_video));
        video_desc->set_rtcp_mux(opts.use_rtcp_mux);
        _local_desc->add_media_desc(video_desc);
        _local_desc->add_transport_info(video_desc->mid(), ice_params);
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