#ifndef _RTCP_SERVER_H_
#define _RTCP_SERVER_H_

#include "udp_connection.h"

class Session;
class Capture;


class RtcpServer : public UdpConnection
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
