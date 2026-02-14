#ifndef __TCP_CONNECTION_H_
#define __TCP_CONNECTION_H_

#include "base/event_loop.h"
#include "base/xhead.h"
#include <rtc_base/sds.h>

namespace xrtc {

class TcpConnection {
public:
    TcpConnection(int fd);
    ~TcpConnection();

public:
    int fd;
    char ip[64];
    int port;
    IOWatcher* _io_watcher = nullptr;
    sds querybuf;// 读取数据的buff
    size_t bytes_expected = XHEAD_SIZE;
    size_t bytes_processed = 0;
};

} // namespace xrtc

#endif // __TCP_CONNECTION_H_
