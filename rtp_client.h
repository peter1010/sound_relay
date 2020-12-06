#ifndef _RTP_CLIENT_H_
#define _RTP_CLIENT_H_

#include "udp_client.h"

class Connection;

class RtpClient : public UdpClient
{
public:
    RtpClient(EventLoop & event_loop);
    virtual ~RtpClient();

    unsigned get_max_recv_len() const {return 0;};

protected:

    static Connection * connection_factory();

private:
};

#endif
