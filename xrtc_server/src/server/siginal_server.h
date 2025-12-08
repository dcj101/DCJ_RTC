#ifndef  __SIGNALING_SERVER_H_
#define  __SIGNALING_SERVER_H_

#include <string>
#include <thread>

// #include "base/event_loop.h"

namespace xrtc {
struct SignalingServerOptions {
    std::string host;
    int port;
    int worker_thread_num;
    int connection_timeout_ms;
};


class SignalingServer {
public:
    SignalingServer();
    ~SignalingServer();

    int init(const char* config_file);
    
};

}

#endif