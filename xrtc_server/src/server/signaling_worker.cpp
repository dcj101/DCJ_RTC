#include "signaling_worker.h"
#include "base/socket.h"
#include "base/xhead.h"
#include "tcp_connection.h"
#include <unistd.h>
#include <rtc_base/logging.h>

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

SignalingServerWorker::SignalingServerWorker(int worker_id) : _worker_id(worker_id), _el(new EventLoop(this)) {

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
    if ((size_t)fd >= _conns.size()) {
        _conns.resize(fd * 2, nullptr);
    }

    _conns[fd] = conn;
}

void SignalingServerWorker::_close_conn(TcpConnection* conn) {
    if (!conn) {
        return;
    }

    int fd = conn->fd;
    if (fd < 0 || fd >= (int)_conns.size()) {
        return;
    }

    if (_conns[fd] == conn) {
        _el->delete_io_event(conn->_io_watcher);
        sdsclear(conn->querybuf);
        sdsfree(conn->querybuf);
        delete conn;
        _conns[fd] = nullptr;
    }
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

}