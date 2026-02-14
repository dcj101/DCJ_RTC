#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <sys/ioctl.h>

#include <rtc_base/logging.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>

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

    ret = bind(sock, (struct sockaddr*)&sa, sizeof(sa));
    if (-1 == ret) {
        RTC_LOG(LS_WARNING) << "bind error, errno: " << errno
            << ", error: " << strerror(errno);
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

int generic_accept(int sock, struct sockaddr* sa, socklen_t* sa_len) {
    int cfd = accept(sock, sa, sa_len);
    while(1) {
        if (-1 == cfd) {
            if (EAGAIN == errno || EWOULDBLOCK == errno) {
                continue;
            } else {
                RTC_LOG(LS_WARNING) << "accept error, errno: " << errno
                    << ", error: " << strerror(errno);
                close(sock);
                return -1;
            }
        }

        break;
    }

    return cfd;
}

int tcp_accept(int fd, char* ip, int* port) {
    struct sockaddr_in sa;
    socklen_t sa_len = sizeof(sa);
    int cfd = generic_accept(fd, (struct sockaddr*)&sa, &sa_len);
    if (-1 == cfd) {
        RTC_LOG(LS_WARNING) << "accept error, errno: " << errno
            << ", error: " << strerror(errno);
        return -1;
    }

    if (ip) {
        inet_ntop(AF_INET, &sa.sin_addr, ip, 128);
    }

    if (port) {
        *port = ntohs(sa.sin_port);
    }

    return cfd;
}

int sock_set_non_block(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        RTC_LOG(LS_WARNING) << "fcntl F_GETFL error, errno: " << errno
            << ", error: " << strerror(errno);
        return -1;
    }

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        RTC_LOG(LS_WARNING) << "fcntl F_SETFL error, errno: " << errno
            << ", error: " << strerror(errno);
        return -1;
    }

    return 0;
}

int sock_set_tcp_nodelay(int fd) {
    int on = 1;
    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on)) < 0) {
        RTC_LOG(LS_WARNING) << "setsockopt TCP_NODELAY error, errno: " << errno
            << ", error: " << strerror(errno);
        return -1;
    }

    return 0;
}

int sock_peer_to_string(int fd, char* ip, int* port) {
    struct sockaddr_in sa;
    socklen_t sa_len = sizeof(sa);
    if (getpeername(fd, (struct sockaddr*)&sa, &sa_len) < 0) {
        RTC_LOG(LS_WARNING) << "getpeername error, errno: " << errno
            << ", error: " << strerror(errno);
        if (ip) {
            memset(ip, 0, sizeof(ip));
        }
        
        if (port) {
            *port = 0;
        }

        return -1;
    }

    if (ip) {
        inet_ntop(AF_INET, &sa.sin_addr, ip, 128);
    }   

    if (port) {
        *port = ntohs(sa.sin_port);
    }

    return 0;
}

}