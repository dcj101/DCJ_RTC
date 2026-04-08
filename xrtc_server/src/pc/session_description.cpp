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
        audio_codec->fb_params.push_back(FeedBackParam("transport-cc"));
        
        audio_codec->codec_params["minptime"] = "10";
        audio_codec->codec_params["useinbandfec"] = "1";

        _codecs.push_back(audio_codec);
}

VideoContentDescription::VideoContentDescription() {
        auto video_codec = std::make_shared<VideoCodecInfo>();
        video_codec->id = 107;
        video_codec->name = "h264";
        video_codec->clock_rate = 90000;// 90kHz

        video_codec->fb_params.push_back(FeedBackParam("goog-remb"));
        video_codec->fb_params.push_back(FeedBackParam("transport-cc"));
        video_codec->fb_params.push_back(FeedBackParam("ccm", "fir"));
        video_codec->fb_params.push_back(FeedBackParam("nack"));
        video_codec->fb_params.push_back(FeedBackParam("nack", "pli"));
        // 视频编码参数
        video_codec->codec_params["level-asymmetry-allowed"] = "1";
        video_codec->codec_params["packetization-mode"] = "1";
        video_codec->codec_params["profile-level-id"] = "42e01";
        _codecs.push_back(video_codec);
        // 重传
        auto rtx_codec = std::make_shared<VideoCodecInfo>();
        rtx_codec->id = 99;
        rtx_codec->name = "rtx";
        rtx_codec->clock_rate = 90000;// 90kHz
        // 关联重传编码id
        rtx_codec->codec_params["apt"] = std::to_string(video_codec->id);
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

// 外部支持添加传输信息
bool SessionDescription::add_transport_info(const std::string& mid, const IceParameters& params, rtc::RTCCertificate* certificate) {
    auto transport = std::make_shared<TransportDescription>();
    transport->mid = mid;
    transport->ice_ufrag = params.ice_ufrag;
    transport->ice_password = params.ice_password;
    if (certificate) {
        transport->identity_fingerprint = rtc::SSLFingerprint::CreateFromCertificate(*certificate);
        if (!transport->identity_fingerprint) {
            RTC_LOG(LS_ERROR) << "create ssl fingerprint failed";
            return false;
        }
    }
    _transports.push_back(transport);
    return true;
}

std::shared_ptr<TransportDescription> SessionDescription::get_transport(const std::string& mid) {
    for (auto& transport : _transports) {
        if (transport->mid == mid) {
            return transport;
        }
    }

    return nullptr;
}

static void add_fmtp_line(std::shared_ptr<CodecInfo> codec, std::stringstream& ss) {
    if (!codec->codec_params.empty()) {
        ss << "a=fmtp:" << codec->id << " ";
        bool isFirst = true;
        for (auto& param : codec->codec_params) {
            if (!isFirst) {
                ss << ";";
            }
            ss << param.first << "=" << param.second;
            isFirst = false;
        }

        ss << "\r\n";
    }
}

static void add_rtcp_fb_line(std::shared_ptr<CodecInfo> codec, std::stringstream& ss) {
    for (auto& fb_param : codec->fb_params) {
        ss << "a=rtcp-fb:" << codec->id << " " << fb_param.id();
        if (!fb_param.param().empty()) {
            ss << " " << fb_param.param();
        }
        ss << "\r\n";
    }
}
static void build_rtp_direction(std::shared_ptr<MediaContentDescription> content, std::stringstream& ss) {
    switch (content->direction())
    {
    case RtpDirection::SendReceive:
        ss << "a=sendrecv\r\n";
        break;
    case RtpDirection::SendOnly:
        ss << "a=sendonly\r\n";
        break;
    case RtpDirection::ReceiveOnly:
        ss << "a=recvonly\r\n";
        break;
    case RtpDirection::Inactive:
        ss << "a=inactive\r\n";
        break;
    default:
        break;
    }
}

static void build_rtp_map(std::shared_ptr<MediaContentDescription> content, std::stringstream& ss) {
    for (auto codec : content->codecs()) {
        ss << "a=rtpmap:" << codec->id << " " << codec->name << "/" << codec->clock_rate;
        if (content->type() == MediaType::AUDIO)  {
            ss << "/" << codec->as_audio()->channels;
        }
        ss << "\r\n";

        add_rtcp_fb_line(codec, ss);
        add_fmtp_line(codec, ss);
    }
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
        ss << "a=rtcp:9 IN IP4 0.0.0.0\r\n";

        ss << "a=mid:" << content->mid() << "\r\n";
        build_rtp_direction(content, ss);
        if (content->rtcp_mux()) {
            ss << "a=rtcp-mux\r\n";
        }
                // 传输信息
        auto transport = get_transport(content->mid());
        if (transport) {
            ss << "a=ice-ufrag:" << transport->ice_ufrag << "\r\n";
            ss << "a=ice-pwd:" << transport->ice_password << "\r\n";
            auto fp = transport->identity_fingerprint.get();
            if (fp) {
                ss << "a=fingerprint:" << fp->algorithm << " " << fp->GetRfc4572Fingerprint() << "\r\n";
            }
        }
        
        build_rtp_map(content, ss);
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