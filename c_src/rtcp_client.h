#ifndef _RTCP_CLIENT_H_
#define _RTCP_CLIENT_H_

#include "udp_client.h"

class UdpConnection;
class Session;
class Capture;


class RtcpClient : public UdpClient
{
public:
    RtcpClient(const Session &);
    virtual ~RtcpClient();

    unsigned get_max_recv_len() const {return 512;};

protected:

    static UdpConnection * connection_factory(void *);

private:

    Capture * mpSource;
//  unsigned char mPayloadType;
};

#endif
