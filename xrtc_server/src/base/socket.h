#ifndef XRTC_SOCKET_H_
#define XRTC_SOCKET_H_
#include <sys/socket.h>
#include <sys/types.h>
namespace xrtc {

int create_tcp_socket(const char* addr, int port);
int tcp_accept(int fd, char* ip, int* port);
int generic_accept(int sock, struct sockaddr* sa, socklen_t* sa_len);
int sock_set_non_block(int fd);
int sock_set_tcp_nodelay(int fd);
int sock_peer_to_string(int fd, char* ip, int* port);
int sock_read_data(int fd, char* data, int len);
int sock_write_data(int sock, const char* buf, size_t len);


}

#endif