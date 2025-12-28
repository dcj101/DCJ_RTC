#include <iostream>
#include "base/conf.h"
#include "base/log.h"
#include "server/siginal_server.h"

xrtc::GeneralConf* conf = nullptr;
xrtc::XrtcLog* g_log = nullptr;
xrtc::SignalingServer* signaling_server = nullptr;


int init_general_conf(const char* filename) {
    if (!filename) {
        fprintf(stderr, "init_general_conf: filename is null\n");
        return -1;
    }
    conf = new xrtc::GeneralConf();
    if (!conf) {
        fprintf(stderr, "init_general_conf: conf is null\n");
        return -1;
    }
    if (load_general_conf(filename, conf) != 0) {
        fprintf(stderr, "init_general_conf: load_general_conf failed\n");
        return -1;
    }

    return 0;
}

int init_log(const std::string& log_dir,
        const std::string& log_name,
        const std::string& log_level) {
    g_log = new xrtc::XrtcLog(log_dir, log_name, log_level);
    if (!g_log) {
        fprintf(stderr, "init_log: log is null\n");
        return -1;
    }
    if (g_log->init() != 0) {
        fprintf(stderr, "init_log: log init failed\n");
        return -1;
    }

    g_log->set_log_to_stderr(conf->log_to_stderr);
    g_log->start();
    return 0;
}

int init_signaling_server() {
    signaling_server = new xrtc::SignalingServer();
    if (!signaling_server) {
        fprintf(stderr, "init_signaling_server: signaling_server is null\n");
        return -1;
    }
    int ret = signaling_server->init("../conf/signaling_server.yaml");
    if (ret != 0) {
        fprintf(stderr, "init_signaling_server: signaling_server init failed\n");
        return -1;
    }
    return 0;
}


int main() {
    int ret = init_general_conf("../conf/general.yaml");
    if (ret != 0) {
        fprintf(stderr, "main: init_general_conf failed\n");
        return -1;
    }
    
    ret = init_log(conf->log_dir, conf->log_name, conf->log_level);
    if (ret != 0) {
        fprintf(stderr, "main: init_log failed\n");
        return -1;
    }

    RTC_LOG(LS_INFO) << "main: log init success";

    ret = init_signaling_server();
    if (ret != 0) {
        fprintf(stderr, "main: init_signaling_server failed\n");
        return -1;
    }
    // signaling_server->start();
        
    g_log->join();
    return 0;
}
