#ifndef _RTCP_CONNECTION_H_
#define _RTCP_CONNECTION_H_

#include <cstdint>
#include <string>
#include "udp_connection.h"


class RtcpConnection : public UdpConnection
{
public:
    RtcpConnection();
    ~RtcpConnection();

    void send_packet();

protected:

    virtual bool parse_recv(const Byte * data, unsigned len);

private:

//    Byte * mPacket;
};

#endif
