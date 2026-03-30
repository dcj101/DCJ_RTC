#ifndef __CODEC_INFO_H_
#define __CODEC_INFO_H_
#include <string>
#include <map>

namespace xrtc {
class AudioCodecInfo;
class VideoCodecInfo;

class FeedBackParam {
public:
    FeedBackParam(const std::string& id, const std::string& param) : _id(id), _param(param) {}
    FeedBackParam(const std::string& id) : _id(id), _param("") {}
    const std::string& id() const { return _id; }
    const std::string& param() const { return _param; }
    
private:
    std::string _id;
    std::string _param;
};

typedef std::map<std::string, std::string> CodecParam;

class CodecInfo {
public:
    virtual ~CodecInfo() = default;
    virtual AudioCodecInfo* as_audio() { return nullptr; }
    virtual VideoCodecInfo* as_video() { return nullptr; }
public:
    int id;
    std::string name;
    int clock_rate;
    // 控制传输质量的feedback rtcp参数
    std::vector<FeedBackParam> fb_params;
    // 控制编码质量的feedback rtcp参数
    CodecParam codec_params;
};

class AudioCodecInfo : public CodecInfo {
public:
    AudioCodecInfo* as_audio() override { return this; }
    int channels;
};

class VideoCodecInfo : public CodecInfo {
public:
    VideoCodecInfo* as_video() override { return this; }
};

}

#endif