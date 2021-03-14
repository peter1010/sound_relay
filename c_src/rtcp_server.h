#ifndef _RTCP_SERVER_H_
#define _RTCP_SERVER_H_

#include "udp_client.h"

class Session;
class Capture;


class RtcpServer : public UdpClient
{
public:
    RtcpServer(const Session &);
    virtual ~RtcpServer();

    unsigned get_max_recv_len() const {return 512;};

protected:

    virtual bool parse_recv(const uint8_t * data, unsigned len);

private:

    Capture * mpSource;
//  unsigned char mPayloadType;
};

#endif
