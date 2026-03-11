#include "session_description.h"
#include <sstream>

namespace xrtc {

const char k_media_protocol_dtls_savpf[] = "UDP/TLS/RTP/SAVPF";
const char k_meida_protocol_savpf[] = "RTP/SAVPF";

AudioContentDescription::AudioContentDescription() {
        auto audio_codec = std::make_shared<AudioCodecInfo>();
        audio_codec->id = 111;
        audio_codec->name = "opus";
        audio_codec->channels = 2;
        audio_codec->clock_rate = 48000;
        _codecs.push_back(audio_codec);
}

VideoContentDescription::VideoContentDescription() {
        auto video_codec = std::make_shared<VideoCodecInfo>();
        video_codec->id = 107;
        video_codec->name = "h264";
        video_codec->clock_rate = 90000;// 90kHz
        _codecs.push_back(video_codec);
        // 重传
        auto rtx_codec = std::make_shared<VideoCodecInfo>();
        rtx_codec->id = 108;
        rtx_codec->name = "rtx";
        rtx_codec->clock_rate = 90000;// 90kHz
        _codecs.push_back(rtx_codec);
}

SessionDescription::SessionDescription(SdpType type) : _type(type) {
}

SessionDescription::~SessionDescription() {
}
// 外部支持添加媒体描述
void SessionDescription::add_media_desc(std::shared_ptr<MediaContentDescription> desc) {
    _media_descs.push_back(desc);
}

std::string SessionDescription::to_string() {
    std::stringstream ss;
    // version
    ss << "v=0\r\n"
    // session origin
    // o=<username = -> <session-id = 1234567890> <session-version = 1234567890> <nettype = IN> <addrtype = IP4> <unicast-address = 127.0.0.1>
       << "o=- 1234567890 1234567890 IN IP4 127.0.0.1\r\n"
    // session name
       << "s=-\r\n"
    // time duration
       << "t=0 0\r\n";
    // bundle 
    const std::vector<const ContentGroup* >& groups = get_group_by_name("BUNDLE");
    if (!groups.empty()) {
        ss << "a=group:" << groups[0]->semantic();
    }
    
    for (auto& group : groups) {
        for (auto& content : group->contents()) {
            ss << " " << content;
        }
        ss << "\r\n";
    }

    ss << "a=msid-semantic: WMS *\r\n";

    for (auto content : _media_descs) {
        // RFC 4566
        // m=<media> <port> <proto> <fmt> ...
        std::string fmt;
        for (auto codec : content->codecs()) {
            fmt.append(" ");
            fmt.append(std::to_string(codec->id));
        }

        ss << "m=" << content->mid() << " 9 " << k_media_protocol_dtls_savpf << fmt << "\r\n";
        ss << "c=IN IP4 0.0.0.0\r\n";
        ss << "a=rtpmap:9 IN IP4 0.0.0.0\r\n";
    }

    return ss.str();
}

const std::vector<const ContentGroup* > SessionDescription::get_group_by_name(const std::string& name) const {
    std::vector<const ContentGroup* > groups;
    for (auto& group : _content_groups) {
        if (group.semantic() == name) {
            groups.push_back(&group);
        }
    }
    return groups;
}

}