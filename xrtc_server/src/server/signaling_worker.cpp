#include "signaling_worker.h"
#include "base/socket.h"
#include "base/xhead.h"
#include "tcp_connection.h"
#include <unistd.h>
#include <rtc_base/logging.h>
#include <json/json.h>
#include "rtc_server.h"

extern xrtc::RtcServer* rtc_server;

namespace xrtc {

void signaling_warker_recv_notify(EventLoop* el, IOWatcher* w, int fd, int events, void* data) {
   int msg;
   int ret = read(fd, &msg, sizeof(msg));
   if (ret != sizeof(msg)) {
        RTC_LOG(LS_ERROR) << "signaling worker " << " recv notify failed, ret: " << ret << " errno: " << strerror(errno);
        return;
   }

   SignalingServerWorker* worker = (SignalingServerWorker*)data;
   worker->_process_notify(msg);
}

void conn_io_cb(EventLoop* el, IOWatcher* w, int fd, int events, void* data) {
    SignalingServerWorker* worker = (SignalingServerWorker*)data;
    if (events & EventLoop::READ) {
        worker->_read_conn(fd);
    }
}

void conn_timer_cb(EventLoop* el, TimerWatcher* w, void* data) {
    TcpConnection* conn = (TcpConnection*)data;
    SignalingServerWorker* worker = (SignalingServerWorker*)el->owner();
    worker->_process_timeout(conn);
}

void SignalingServerWorker::_process_timeout(TcpConnection* c) {
    //检查链接是否超时
    if (_el->now() - c->last_active_time >= (unsigned int)_options.connection_timeout_ms) {
        RTC_LOG(LS_INFO) << "signaling worker " << _worker_id << " conn " << c->fd << " timeout";
        _close_conn(c);
    }

}

SignalingServerWorker::SignalingServerWorker(int worker_id, const SignalingServerOptions& options) : _worker_id(worker_id), _el(new EventLoop(this)), _options(options) {

}

SignalingServerWorker::~SignalingServerWorker() {
    for (auto conn : _conns) {
        if (conn) {
            _close_conn(conn);
        }
    }

    _conns.clear();

    if (_el) {
        delete _el;
        _el = nullptr;
    }

    close(_notify_recv_fd);
    close(_notify_send_fd);

    if (_thread) {
        _thread->join();
        delete _thread;
        _thread = nullptr;
    }
}



int SignalingServerWorker::init() {

    int fds[2];
    if (pipe(fds) < 0) {
        RTC_LOG(LS_ERROR) << "signaling worker " << _worker_id << " create pipe failed errno: " << strerror(errno);
        return -1;
    }
    _notify_recv_fd = fds[0];
    _notify_send_fd = fds[1];
    
    _pipe_watcher = _el->create_io_event(signaling_warker_recv_notify, this);
    _el->start_io_event(_pipe_watcher, _notify_recv_fd, EventLoop::READ);
}

bool SignalingServerWorker::start() {
    if (_thread) {
        RTC_LOG(LS_ERROR) << "signaling worker " << _worker_id << " start failed, thread already exist";
        return false;
    }

    _thread = new std::thread([=]() {
        RTC_LOG(LS_INFO) << "signaling worker " << _worker_id << " start";
        _el->start();
        RTC_LOG(LS_INFO) << "signaling worker " << _worker_id << " stop";
    });

    return true;
}

void SignalingServerWorker::stop() {
    notify(QUIT);
}

int SignalingServerWorker::notify(int msg) {
    int ret = write(_notify_send_fd, &msg, sizeof(msg));
    if (ret != sizeof(msg)) {
        RTC_LOG(LS_ERROR) << "signaling worker " << _worker_id << " notify failed, ret: " << ret << " errno: " << strerror(errno);
        return -1;
    }

    return 0;
}

void SignalingServerWorker::_process_notify(int msg) {
    RTC_LOG(LS_INFO) << "signaling worker " << _worker_id << " recv notify msg: " << msg;
    switch (msg) {
        case QUIT:
            _stop();
            break;
        case NEW_CONN:
            int fd;
            if (_q_conn.consume(&fd)) {
                _new_conn(fd);
            }

            break;
        case RTC_MSG:
           _process_rtc_msg();
            break;
        default:
            RTC_LOG(LS_ERROR) << "signaling worker " << _worker_id << " unknown notify msg: " << msg;
            break;
    }
}

void SignalingServerWorker::_new_conn(int fd) {
    RTC_LOG(LS_INFO) << "signaling worker " << _worker_id << " new conn fd: " << fd;
    
    if (fd < 0) {
        RTC_LOG(LS_ERROR) << "signaling worker " << _worker_id << " new conn failed, fd: " << fd;
        return;
    }

    if (sock_set_non_block(fd) < 0 || sock_set_tcp_nodelay(fd) < 0) {
        RTC_LOG(LS_ERROR) << "signaling worker " << _worker_id << " new conn failed, fd: " << fd;
        close(fd);
        return;
    }

    TcpConnection* conn = new TcpConnection(fd);
    if (sock_peer_to_string(fd, conn->ip, &conn->port) < 0) {
        RTC_LOG(LS_ERROR) << "signaling worker " << _worker_id << " new conn failed, fd: " << fd;
        close(fd);
        return;
    }

    conn->_io_watcher = _el->create_io_event(conn_io_cb, this);
    _el->start_io_event(conn->_io_watcher, fd, EventLoop::READ);
    conn->timer_watcher = _el->create_timer(conn_timer_cb, conn, true);
    _el->start_timer(conn->timer_watcher, 100000); // 100ms
    conn->last_active_time = _el->now();

    if ((size_t)fd >= _conns.size()) {
        _conns.resize(fd * 2, nullptr);
    }

    _conns[fd] = conn;
}

void SignalingServerWorker::_close_conn(TcpConnection* c) {
    RTC_LOG(LS_INFO) << "close connection, fd: " << c->fd;
    close(c->fd);
    _remove_conn(c);
}

void SignalingServerWorker::_remove_conn(TcpConnection* c) {
    _el->delete_timer(c->timer_watcher);
    _el->delete_io_event(c->_io_watcher);
    _conns[c->fd] = nullptr;
    delete c;
}

void SignalingServerWorker::_read_conn(int fd) {
    RTC_LOG(LS_INFO) << "signaling worker " << _worker_id << " read conn fd: " << fd;
    if (fd < 0 || fd >= (int)_conns.size() || !_conns[fd]) {
        RTC_LOG(LS_ERROR) << "signaling worker " << _worker_id << " read conn failed, fd: " << fd;
        return;
    }

    TcpConnection* conn = _conns[fd];
    // 本地读了多少
    int nread = 0;
    // 期待读大小
    int read_len = conn->bytes_expected;
    // 获取里面存储了多少数据
    int qb_len = sdslen(conn->querybuf);
    // 看看需不需要重新分配空间
    conn->querybuf = sdsMakeRoomFor(conn->querybuf, read_len);
    nread = sock_read_data(fd, conn->querybuf + qb_len, read_len);
    conn->last_active_time = _el->now();
    RTC_LOG(LS_INFO) << "sock read data, len: " << nread;

     if (-1 == nread) {
        _close_conn(conn);
        return;
    } else if (nread > 0) {
        sdsIncrLen(conn->querybuf, nread);
    }

    int ret = _process_query_buffer(conn);
    if (ret != 0) {
        _close_conn(conn);
        return;
    } else {

    }

}

int SignalingServerWorker::_process_query_buffer(TcpConnection* c) {
    // 判断至少读入了一个头部
    while(sdslen(c->querybuf) >= c->bytes_expected + c->bytes_processed) {
        xhead_t* head = (xhead_t*) c->querybuf;
        if (TcpConnection::STATE_HEAD == c->currentState) {
            if (XHEAD_MAGIC_NUM != head->magic_num) {
                RTC_LOG(LS_WARNING) << "invaild data, fd:" << c->fd << " magic num " << head->magic_num;
                return -1;
            }

            c->currentState = TcpConnection::STATE_BODY;
            c->bytes_processed = XHEAD_SIZE;
            c->bytes_expected = head->body_len;
        } else {
            rtc::Slice header(c->querybuf,XHEAD_SIZE);
            rtc::Slice body(c->querybuf + XHEAD_SIZE, head->body_len);

            int ret = _process_request(c, header, body);
            if (ret != 0) {
                return -1;
            }

            // 短链接处理
            c->bytes_processed = 65535;
        }
    }

    return 0;
}

int SignalingServerWorker::_process_request(TcpConnection* c, const rtc::Slice& header, const rtc::Slice& body) {
    RTC_LOG(LS_WARNING) << "recv head body : " << body.data();
    xhead_t* head = (xhead_t*) header.data();
    uint32_t log_id = head->log_id;

    Json::CharReaderBuilder builder;
    std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    Json::Value root;
    JSONCPP_STRING errs;
    if (!reader->parse(body.data(), body.data() + body.size(), &root, &errs)) {
        RTC_LOG(LS_WARNING) << "parse json body error, fd: " << c->fd << ", err: " << errs << ", log_id: " << log_id;
        return -1;
    }

    int cmdno;
    try {
        cmdno = root["cmdno"].asInt();
    } catch (Json::Exception e) {
        RTC_LOG(LS_WARNING) << "no cmdno field in body, log_id: " << log_id;
        return -1;
    }

    switch (cmdno)
    {
    case CMDNO_PUSH:
        return _process_push(cmdno, c, root, log_id);
    case CMDNO_PULL:
        /* code */
        break;
    
    default:
        break;
    }
}

int SignalingServerWorker::_process_push(int cmdno, TcpConnection* c, const Json::Value& root, int log_id)
{
    uint64_t uid;
    std::string stream_name;
    int audio;
    int video; 
    try {
        uid = root["uid"].asUInt64();
        stream_name = root["stream_name"].asString();
        audio = root["audio"].asInt();
        video = root["video"].asInt();
    } catch (Json::Exception e) {
        RTC_LOG(LS_WARNING) << "parse json body error : " << e.what()
         << "log_id: " << log_id;
        return -1;
    }

    RTC_LOG(LS_INFO) << "cmdno[" << cmdno 
    << "] uid[" << uid 
    << "] stream_name[" << stream_name 
    << "] audio[" << audio 
    << "] video[" << video << "]" 
    << "push request, log_id: " << log_id;
    
std::shared_ptr<RtcMsg> msg = std::make_shared<RtcMsg>();
    msg->cmdno = cmdno;
    msg->stream_name = stream_name;
    msg->audio = audio;
    msg->video = video;
    msg->uid = uid;
    msg->log_id = log_id;
    msg->woeker = this;
    msg->conn = c;
    
    return rtc_server->send_rtc_msg(msg);
}

void SignalingServerWorker::_stop() {
    if (!_thread) {
        RTC_LOG(LS_ERROR) << "signaling worker " << _worker_id << " stop failed, thread not exist";
        return;
    }

    _el->delete_io_event(_pipe_watcher);
    _el->stop();

    close(_notify_recv_fd);
    close(_notify_send_fd);

    RTC_LOG(LS_INFO) << "signaling worker " << _worker_id << " stop";
}

void SignalingServerWorker::join() {
    if (_thread && _thread->joinable()) {
        _thread->join();
    }
}


int SignalingServerWorker::notify_new_conn(int fd) {
    _q_conn.produce(fd);
    return notify(NEW_CONN);
}

int SignalingServerWorker::send_rtc_msg(std::shared_ptr<RtcMsg> msg) {
    push_rtc_msg(msg);
    return 0;
}

void SignalingServerWorker::push_rtc_msg(std::shared_ptr<RtcMsg> msg) {
    std::lock_guard<std::mutex> lck(_q_rtc_msg_mtx);
    _q_rtc_msg.push(msg);
}

std::shared_ptr<RtcMsg> SignalingServerWorker::pop_rtc_msg() {
    std::lock_guard<std::mutex> lck(_q_rtc_msg_mtx);
    if (_q_rtc_msg.empty()) {
        return nullptr;
    } 

    std::shared_ptr<RtcMsg> msg = _q_rtc_msg.front();
    _q_rtc_msg.pop();
    return msg;
}

int SignalingServerWorker::_process_rtc_msg() {
    std::shared_ptr<RtcMsg> msg = pop_rtc_msg();
    if (!msg) {
        return 0;
    }

    switch (msg->cmdno)
    {
    case CMDNO_PUSH:
        _response_server_offer(msg);
        break;
    
    default:
        RTC_LOG(LS_ERROR) << "signaling worker " << _worker_id << " unknown rtc msg cmdno: " << msg->cmdno << ", log_id: " << msg->log_id;
        break;
    }
}

void SignalingServerWorker::_response_server_offer(std::shared_ptr<RtcMsg> msg) {
    RTC_LOG(LS_INFO) << "signaling worker " << _worker_id << " response server offer, log_id: " << msg->log_id;
}

}