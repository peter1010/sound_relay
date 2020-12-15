#ifndef _RTP_CLIENT_H_
#define _RTP_CLIENT_H_

#include "udp_client.h"

class Connection;
class Session;
class Capture;


class RtpClient : public UdpClient
{
public:
    RtpClient(const Session &);
    virtual ~RtpClient();

    unsigned get_max_recv_len() const {return 0;};

protected:

    static Connection * connection_factory(void *);

private:

    Capture * mpSource;
};

#endif
