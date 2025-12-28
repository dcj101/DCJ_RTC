#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>

#include <rtc_base/logging.h>

#include "base/socket.h"

namespace xrtc {

int create_tcp_socket(const char* addr, int port) {
    // 1. 创建socket

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == sock) {
        RTC_LOG(LS_WARNING) << "create socket error, errno: " << errno
            << ", error: " << strerror(errno);
        return -1;
    }

    // 2. 设置SO_REUSEADDR
    int on = 1;
    int ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    if (-1 == ret) {
        RTC_LOG(LS_WARNING) << "setsockopt SO_REUSEADDR error, errno: " << errno
            << ", error: " << strerror(errno);
        close(sock);
        return -1;
    }

    // 3. 创建addr
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);

    if (addr && inet_aton(addr, &sa.sin_addr) == 0) {
        RTC_LOG(LS_WARNING) << "invalid address";;
        close(sock);
        return -1;
    }

 // 设置监听队列的最大长度
    ret = listen(sock, 4096);
    if (-1 == ret) {
        RTC_LOG(LS_WARNING) << "listen error, errno: " << errno
            << ", error: " << strerror(errno);
        close(sock);
        return -1;
    }

    return sock;
}


}