#ifndef _RTSP_SERVER_H_
#define _RTSP_SERVER_H_

#include "tcp_server.h"

class Connection;

class RtspServer : public TcpServer
{
public:
    RtspServer();

    virtual ~RtspServer();

    unsigned get_max_recv_len() const {return 1024;};

protected:

    static Connection * connection_factory();

private:
};

#endif
