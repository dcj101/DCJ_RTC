#ifndef __SESSION_DESCRIPTION_H_
#define __SESSION_DESCRIPTION_H_
#include <string>
#include <vector>
#include <memory>
#include "codec_info.h"
#include "rtc_base/rtc_certificate.h"
#include "rtc_base/ssl_fingerprint.h"
#include "ice/ice_credentials.h"
#include "base/log.h"



namespace xrtc {

enum class SdpType {
    OFFER = 0,
    ANSWER = 1,
};

enum class MediaType {
    AUDIO = 0,
    VIDEO = 1,
};

enum class RtpDirection {
    SendOnly = 0,
    ReceiveOnly = 1,
    SendReceive = 2,
    Inactive = 3,
};

class MediaContentDescription {
public:
    MediaContentDescription() = default;
    virtual ~MediaContentDescription() = default;
    void set_direction(RtpDirection dir) { _direction = dir; }
    virtual MediaType type() = 0;
    virtual std::string mid() = 0;
    const std::vector<std::shared_ptr<CodecInfo>>& codecs() const { return _codecs; }
    RtpDirection direction() const { return _direction; }
    bool rtcp_mux() const { return _rtcp_mux; }
    bool set_rtcp_mux(bool mux) { _rtcp_mux = mux; }
protected:
    std::vector<std::shared_ptr<CodecInfo>> _codecs;
    RtpDirection _direction = RtpDirection::Inactive;
    bool _rtcp_mux = true;
};

class AudioContentDescription : public MediaContentDescription {
public:
    AudioContentDescription();

    MediaType type() override { return MediaType::AUDIO; };
    std::string mid() override { return "audio"; };
};

class VideoContentDescription : public MediaContentDescription {
public:
    VideoContentDescription();
    MediaType type() override { return MediaType::VIDEO; };
    std::string mid() override { return "video"; };
};

class ContentGroup {
public:
    ContentGroup(const std::string& semantic) : _semantic(semantic) {};
    ~ContentGroup() = default;
    bool has_content(const std::string& content) { return std::find(_contents.begin(), _contents.end(), content) != _contents.end(); }
    void add_content(const std::string& content) { if (!has_content(content)) _contents.push_back(content); }
    std::string semantic() const { return _semantic; }
    const std::vector<std::string>& contents() const { return _contents; }
private:
    std::string _semantic;
    std::vector<std::string> _contents;
};

class TransportDescription {
public:
    std::string mid;
    std::string ice_ufrag;
    std::string ice_password;
    std::unique_ptr<rtc::SSLFingerprint> identity_fingerprint;
};

class SessionDescription {
public:
    SessionDescription(SdpType type);
    ~SessionDescription();
    std::string to_string();
    void add_media_desc(std::shared_ptr<MediaContentDescription> desc);
    const std::vector<std::shared_ptr<MediaContentDescription>>& media_descs() const { return _media_descs; }
    void add_content_group(const ContentGroup& group) { _content_groups.push_back(group); }
    const std::vector<ContentGroup>& content_groups() const { return _content_groups; }
    bool add_transport_info(const std::string& mid, const IceParameters& params, rtc::RTCCertificate* certificate);
    std::shared_ptr<TransportDescription> get_transport(const std::string& mid);
private:
    const std::vector<const ContentGroup* > get_group_by_name(const std::string& name) const;
private:
    std::string _sdp;
    SdpType _type;
    std::vector<std::shared_ptr<MediaContentDescription>> _media_descs;
    std::vector<ContentGroup> _content_groups;
    std::vector<std::shared_ptr<TransportDescription>> _transports;
};

}
#endif