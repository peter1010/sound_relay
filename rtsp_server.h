#ifndef _RTSP_SERVER_H_
#define _RTSP_SERVER_H_

#include "tcp_server.h"


class RtspServer : public TcpServer
{
public:
    RtspServer(EventLoop & event_loop);

private:

    bool parse_recv(TcpConnection & rConn);
};

#endif
