#include "siginal_server.h"
#include "base/log.h"
#include "base/socket.h"
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
        RTC_LOG(LS_INFO) << "SignalingServer::init config\n" << config;
        _options.host = config["host"].as<std::string>();
        _options.port = config["port"].as<int>();
        _options.worker_thread_num = config["worker_num"].as<int>();
        _options.connection_timeout_ms = config["connection_timeout"].as<int>();
    } catch (const YAML::Exception& e) {
        RTC_LOG(LS_ERROR) << "SignalingServer::init config_file: " << config_file << " error: " << e.what();
        return -1;
    }

    // 创建tcp server
    _listen_fd = create_tcp_socket(_options.host.c_str(), _options.port); 
    if (-1 == _listen_fd) {
        return -1;
    }

    return 0;
}



}