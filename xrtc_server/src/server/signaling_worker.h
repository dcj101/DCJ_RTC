#ifndef __SIGNALING_WORKER_H_
#define __SIGNALING_WORKER_H_
#include "base/event_loop.h"
#include "base/lock_free_queue.h"
#include <thread>
#include <rtc_base/slice.h>

namespace xrtc {
class TcpConnection;
class SignalingServerWorker {
public:
    enum {
        QUIT = 0,
        NEW_CONN = 1,
    };
    SignalingServerWorker(int worker_id);
    ~SignalingServerWorker() = default;
    int init();
    bool start();
    void stop();
    void join();
    int notify_new_conn(int fd);
private:
    friend void signaling_warker_recv_notify(EventLoop* el, IOWatcher* w, int fd, int events, void* data);
    friend void conn_io_cb(EventLoop* el, IOWatcher* w, int fd, int events, void* data);
    int notify(int msg);
    void _process_notify(int msg);
    void _stop();
    void _new_conn(int fd);
    void _read_conn(int fd);
    void _close_conn(TcpConnection* c);
    int _process_query_buffer(TcpConnection* c);
    int _process_request(TcpConnection* c, const rtc::Slice& header, const rtc::Slice& body);
private:
    int _worker_id;
    EventLoop* _el;
    IOWatcher* _pipe_watcher = nullptr;
    int _notify_recv_fd = -1;
    int _notify_send_fd = -1;
    std::thread* _thread = nullptr;
    LockFreeQueue<int> _q_conn;
    std::vector<TcpConnection*> _conns;
};

}

#endif