#include "tcp_connection.h"
#include <unistd.h>
#include <rtc_base/logging.h>

namespace xrtc {

TcpConnection::TcpConnection(int fd) : fd(fd),querybuf(sdsempty()) {
    memset(ip, 0, sizeof(ip));
    port = 0;
}

TcpConnection::~TcpConnection() {
    close(fd);
}



} // namespace xrtc
