#ifndef  __SIGNALING_SERVER_H_
#define  __SIGNALING_SERVER_H_

#include <string>
#include <thread>
#include <vector>
#include "base/event_loop.h"


namespace xrtc {

class SignalingServerWorker;

struct SignalingServerOptions {
    std::string host;
    int port;
    int worker_thread_num;
    int connection_timeout_ms;
};


class SignalingServer {
public:
    enum {
        QUIT = 0,
    };
    SignalingServer();
    ~SignalingServer();
    bool start();
    void stop();
    int init(const char* config_file);
    bool notify(int msg);

    void join();
    friend void signaling_server_recv_notify(EventLoop* el, IOWatcher* w, 
        int fd, int events, void* data);

    friend void accept_new_conn(EventLoop* el, IOWatcher* w, 
            int fd, int events, void* data);
private:
    void _process_notify(int msg);
    void _stop();
    int _create_worker(int worker_id);
    void _dispatch_new_conn(int fd);
private:
    SignalingServerOptions _options;
    // 要把fd放到event loop 里面管理
    int _listen_fd = -1;
    IOWatcher* _io_watcher = nullptr;
    IOWatcher* _pipe_watcher = nullptr;
    int _notify_recv_fd = -1;
    int _notify_send_fd = -1;
    EventLoop* _el;
    std::thread* _thread = nullptr;
    std::vector<SignalingServerWorker*> _workers;
    int _next_worker_id = 0;
};

}

#endif