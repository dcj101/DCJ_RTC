#include "session_description.h"
#include <sstream>

namespace xrtc {
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