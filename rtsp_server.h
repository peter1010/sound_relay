#ifndef _RTSP_SERVER_H_
#define _RTSP_SERVER_H_

#include "tcp_server.h"

class TcpConnection;

class RtspServer : public TcpServer
{
public:
    RtspServer(EventLoop & event_loop);

private:

    bool parse_recv(TcpConnection & rConn);

    unsigned getMaxRecvBufLen() const {return 1024;};

    void parse_request(TcpConnection & rConn);

    void parse_line(TcpConnection & rConn, const char * pBuf, unsigned len);
};

#endif
