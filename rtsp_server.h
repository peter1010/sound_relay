#ifndef _RTSP_SERVER_H_
#define _RTSP_SERVER_H_

#include "tcp_server.h"

class TcpConnection;

class RtspServer : public TcpServer
{
public:
    RtspServer(EventLoop & event_loop);

    ~RtspServer();

    unsigned get_max_recv_len() const {return 1024;};

protected:

    static TcpConnection * connection_factory(int, TcpServer &);

private:
};

#endif
