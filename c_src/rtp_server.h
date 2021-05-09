#ifndef _RTP_SERVER_H_
#define _RTP_SERVER_H_

#include "udp_connection.h"

class Session;
class Replay;


class RtpServer : public UdpConnection
{
public:
	RtpServer(const Session &);
	virtual ~RtpServer();

	// Methods called from the Replay class
	uint8_t * get_packet_buffer() const { return &mPacket[RTP_HEADER_LEN]; };

	unsigned get_packet_buffer_size() const { return 0; }; // return MAX_PKT_SIZE - RTP_HEADER_LEN; };

	void send_packet(unsigned num_to_send, unsigned timeDuration);

protected:

	virtual bool parse_recv(const uint8_t * data, unsigned len);

	virtual unsigned get_max_recv_len() const {return MAX_PKT_SIZE;};

private:

	Replay * mpSink;

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
};


#endif
