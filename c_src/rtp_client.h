#ifndef _RTP_CLIENT_H_
#define _RTP_CLIENT_H_

#include "udp_connection.h"

class Session;
class Capture;


class RtpClient : public UdpConnection
{
public:
	RtpClient(const Session & session);
	virtual ~RtpClient();

	// Methods called from the Capture class
	uint8_t * get_packet_buffer() const { return &mPacket[RTP_HEADER_LEN]; };

	unsigned get_packet_buffer_size() const { return MAX_PKT_SIZE - RTP_HEADER_LEN; };

	void send_packet(unsigned num_to_send, unsigned timeDuration);

protected:

	virtual bool parse_recv(const uint8_t * data, unsigned len);

	virtual unsigned get_max_recv_len() const {return 0;};

private:

	Capture * mpSource;

	enum {
		RTP_VERSION = 2,
		RTP_PADDING = 0,
		RTP_EXTENSION = 0,
		RTP_CSRC_COUNT = 0,

		MAX_PKT_SIZE = 2048,

		RTP_HEADER_LEN = 12 + RTP_CSRC_COUNT * 4
   };

	uint8_t * mPacket;
	uint16_t mSequenceNumber;
	uint32_t mTimeStamp;
	uint32_t mSendFails;
};


#endif
