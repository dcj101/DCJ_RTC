#include "rtc_stream.h"

namespace xrtc {


RtcStream::RtcStream(EventLoop* el, uint64_t uid, const std::string& stream_name,
        bool audio, bool video, uint32_t log_id)
    : _uid(uid), _stream_name(stream_name), _audio(audio), _video(video),
      _log_id(log_id), _el(el) {
    _pc = std::make_unique<PeerConnection>(_el);
}

RtcStream::~RtcStream() {
    
}

int RtcStream::start(rtc::RTCCertificate* certificate) {
    return _pc->init(certificate);
}


}