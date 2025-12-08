#include "siginal_server.h"
#include "base/log.h"
#include <yaml-cpp/yaml.h>

namespace xrtc {
SignalingServer::SignalingServer() {
}

SignalingServer::~SignalingServer() {
}


int SignalingServer::init(const char* config_file) {
    if (!config_file) {
        RTC_LOG(LS_WARNING) << "SignalingServer::init config_file is null";
        return -1;
    }

    try {
        YAML::Node config = YAML::LoadFile(config_file);
        RTC_LOG(LS_INFO) << "SignalingServer::init config_file: " << config_file;
    } catch (const YAML::Exception& e) {
        RTC_LOG(LS_ERROR) << "SignalingServer::init config_file: " << config_file << " error: " << e.what();
        return -1;
    }


}



}