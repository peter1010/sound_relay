#ifndef _RTP_CONNECTION_H_
#define _RTP_CONNECTION_H_

#include <string>
#include "udp_connection.h"


class RtpConnection : public UdpConnection
{
public:
    RtpConnection();

protected:

    virtual bool parse_recv(const Byte * data, unsigned len);

private:
};

#endif
