#ifndef _RTCP_CLIENT_H_
#define _RTCP_CLIENT_H_

#include "udp_connection.h"

class Session;
class Replay;


class RtcpClient : public UdpConnection
{
public:
	RtcpClient(const Session &);
	virtual ~RtcpClient();

	unsigned get_max_recv_len() const {return 0;};

protected:

	virtual bool parse_recv(const uint8_t * data, unsigned len);

private:

	Replay * mpSource;
//  unsigned char mPayloadType;
};

#endif
