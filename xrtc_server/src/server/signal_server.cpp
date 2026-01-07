#include "signal_server.h"
#include "base/log.h"
#include "base/socket.h"
#include <yaml-cpp/yaml.h>
#include <unistd.h>

namespace xrtc {

void accept_new_conn(EventLoop* /*el*/, IOWatcher* /*w*/, int fd, int /*events*/, void* data) {
    
}

void signaling_server_recv_notify(EventLoop* /*el*/, IOWatcher* /*w*/, 
        int fd, int /*events*/, void* data) 
{
    int msg;
    if (read(fd, &msg, sizeof(int)) != sizeof(int)) {
        RTC_LOG(LS_WARNING) << "read from pipe error: " << strerror(errno)
            << ", errno: " << errno;
        return;
    }

    SignalingServer* server = (SignalingServer*)data;
    server->_process_notify(msg);
}

SignalingServer::SignalingServer() : _el(new EventLoop(this)) {
}

SignalingServer::~SignalingServer() {
}


int SignalingServer::init(const char* config_file) {
    if (!config_file) {
        RTC_LOG(LS_WARNING) << "SignalingServer::init config_file is null";
        return -1;
    }
    // 获取配置
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

    // 创建pipe 用于通知event loop 退出
    int fds[2];
    if (-1 == pipe(fds)) {
        RTC_LOG(LS_WARNING) << "signaling server::start create pipe error, errno: " << errno
            << ", error: " << strerror(errno);
        return -1;
    }
    _notify_recv_fd = fds[0];
    _notify_send_fd = fds[1];

    // 接受消息的fd放到事件循环的监听列表里面
    _pipe_watcher = _el->create_io_event(signaling_server_recv_notify, this);
    _el->start_io_event(_pipe_watcher, _notify_recv_fd, EventLoop::READ);

    // 创建tcp server
    _listen_fd = create_tcp_socket(_options.host.c_str(), _options.port); 
    _io_watcher = _el->create_io_event(accept_new_conn, this);
    // 监听新连接的fd放到事件循环的监听列表里面
    _el->start_io_event(_io_watcher, _listen_fd, EventLoop::READ);
    if (-1 == _listen_fd) {
        return -1;
    }

    return 0;
}

bool SignalingServer::start() {
    if (_thread) {
        RTC_LOG(LS_WARNING) << "signaling server::start thread is already running";
        return false;
    }

    _thread = new std::thread([=]() {
        RTC_LOG(LS_INFO) << "event loop thread is running";
        _el->start();
        RTC_LOG(LS_INFO) << "event loop thread is stopped";
    });
}

void SignalingServer::stop() {
    notify(QUIT);
}

bool SignalingServer::notify(int msg) {
    int written = write(_notify_send_fd, &msg, sizeof(msg));
    return written == sizeof(msg) ? 0 : -1;
}

// 保证_stop只在_thread 线程调用
void SignalingServer::_process_notify(int msg) {
    switch (msg) {
        case QUIT:
            _stop();
            break;
        default:
            RTC_LOG(LS_WARNING) << "unknown msg: " << msg;
            break;
    }
}

void SignalingServer::_stop() {
    if (!_thread) {
        RTC_LOG(LS_WARNING) << "signaling server not running";
        return;
    }

    _el->delete_io_event(_pipe_watcher);
    _el->delete_io_event(_io_watcher);
    _el->stop();

    close(_notify_recv_fd);
    close(_notify_send_fd);
    close(_listen_fd);

    RTC_LOG(LS_INFO) << "signaling server stop";

    // for (auto worker : _workers) {
    //     if (worker) {
    //         worker->stop();
    //         worker->join();
    //     }
    // }
}

void SignalingServer::join() {
    if (_thread && _thread->joinable()) {
        _thread->join();
    }
}

}