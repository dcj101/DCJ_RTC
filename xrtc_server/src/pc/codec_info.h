#ifndef __CODEC_INFO_H_
#define __CODEC_INFO_H_
#include <string>

namespace xrtc {
class CodecInfo {
public:
    virtual ~CodecInfo() = default;
public:
    int id;
    std::string name;
    int clock_rate;
};

class AudioCodecInfo : public CodecInfo {
public:
    int channels;
};

class VideoCodecInfo : public CodecInfo {
public:
};

}

#endif