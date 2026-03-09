#ifndef __SIGNALING_WORKER_H_
#define __SIGNALING_WORKER_H_
#include "base/event_loop.h"
#include "base/lock_free_queue.h"
#include <thread>
#include <rtc_base/slice.h>
#include "signaling_server.h"
#include <json/json.h>
#include "base/xrtcserver_def.h"
#include <mutex>
#include <queue>


namespace xrtc {
class TcpConnection;
class SignalingServerWorker {
public:
    enum {
        QUIT = 0,
        NEW_CONN = 1,
        RTC_MSG = 2,
    };
    SignalingServerWorker(int worker_id, const SignalingServerOptions& options);
    ~SignalingServerWorker();
    int init();
    bool start();
    void stop();
    void join();
    int notify_new_conn(int fd);
    int send_rtc_msg(std::shared_ptr<RtcMsg> msg);
    void push_rtc_msg(std::shared_ptr<RtcMsg> msg);
    std::shared_ptr<RtcMsg> pop_rtc_msg();
private:
    friend void signaling_warker_recv_notify(EventLoop* el, IOWatcher* w, int fd, int events, void* data);
    friend void conn_io_cb(EventLoop* el, IOWatcher* w, int fd, int events, void* data);
    friend void conn_timer_cb(EventLoop* el, TimerWatcher* w, void* data);
    int notify(int msg);
    void _process_notify(int msg);
    void _stop();
    void _new_conn(int fd);
    void _read_conn(int fd);
    void _close_conn(TcpConnection* c);
    int _process_query_buffer(TcpConnection* c);
    int _process_request(TcpConnection* c, const rtc::Slice& header, const rtc::Slice& body);
    void _process_timeout(TcpConnection* c);
    void _remove_conn(TcpConnection* c);
    int _process_push(int cmdno, TcpConnection* c, const Json::Value& root, int log_id);
    int _process_rtc_msg();
    void _response_server_offer(std::shared_ptr<RtcMsg> msg);
    void _add_reply(TcpConnection* c, const rtc::Slice& res_msg);
    void _write_conn(int fd);
private:
    int _worker_id;
    EventLoop* _el;
    IOWatcher* _pipe_watcher = nullptr;
    int _notify_recv_fd = -1;
    int _notify_send_fd = -1;
    std::thread* _thread = nullptr;
    LockFreeQueue<int> _q_conn;
    std::vector<TcpConnection*> _conns;
    SignalingServerOptions _options;
    // chuli1rtc的消息队列
    std::mutex _q_rtc_msg_mtx;
    std::queue<std::shared_ptr<RtcMsg>> _q_rtc_msg;
};

}

#endif