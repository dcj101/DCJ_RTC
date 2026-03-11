#include "tcp_connection.h"
#include <unistd.h>
#include <rtc_base/logging.h>
#include <rtc_base/zmalloc.h>

namespace xrtc {

TcpConnection::TcpConnection(int fd) : fd(fd),querybuf(sdsempty()) {
    memset(ip, 0, sizeof(ip));
    port = 0;
}

TcpConnection::~TcpConnection() {
    close(fd);
    sdsfree(querybuf);
    while (!reply_msgs.empty()) {
        auto msg = reply_msgs.front();
        zfree((void *)msg.data());
        reply_msgs.pop_front();
    }

    reply_msgs.clear();
    
}



} // namespace xrtc
